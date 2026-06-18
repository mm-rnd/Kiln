#pragma once

#include <juce_dsp/juce_dsp.h>
#include <vector>

/**
 * A VCA/optical-style compressor that processes multi-channel audio.
 *
 * Accepts per-channel const float* pointers (e.g. from LinearPhaseCrossover
 * band getters) and writes compressed output to user-supplied float* buffers.
 * The number of channels is configurable at runtime via prepare(), making it
 * channel-agnostic.
 *
 * Architecture (feed-forward):
 *   1. Delay line           (fixed 2 ms, always on — keeps all bands aligned)
 *   2. RMS envelope detection  (IIR lowpass on squared signal)
 *   3. Gain computer           (threshold, ratio, soft/hard knee)
 *   4. Ballistics              (attack/release smoothing on gain reduction)
 *   5. Makeup gain
 *
 * The fixed 2 ms delay ensures all compressor instances have identical
 * latency, keeping crossover bands phase-aligned when summed.  The lookahead
 * toggle switches between:
 *
 *   - Lookahead OFF: envelope reads the delayed (output) sample
 *                     → gain reduction reacts to the signal being output
 *   - Lookahead ON:  envelope reads the current (input) sample
 *                     → gain reduction precedes the delayed output
 *
 * All float parameters are smoothly ramped via juce::SmoothedValue to
 * prevent zipper noise when parameters change.  Call prepare() before first
 * use, and whenever the sample rate, block size or channel count changes.
 */
class Compressor
{
  public:
	Compressor() = default;

	/** Initialise internal state. Must be called before process(). */
	void prepare(double sampleRate, int blockSize, int numChannels);

	/**
	 * Process one block of audio.
	 *
	 * @param inputChannels  Array of const float* pointers, one per channel.
	 *                       These are the input signals (e.g. from a crossover
	 *                       band).
	 * @param outputChannels Array of float* pointers, one per channel, to
	 *                       write the compressed output into.
	 * @param numSamples     Number of samples in each channel.
	 */
	void process(const float* const* inputChannels, float* const* outputChannels, int numSamples);

	// -----------------------------------------------------------------------
	//  Parameter setters
	// -----------------------------------------------------------------------

	/** Attack time in milliseconds (typical range: 0.1 – 100 ms). */
	void setAttack(float ms);

	/** Release time in milliseconds (typical range: 10 – 1000 ms). */
	void setRelease(float ms);

	/** Threshold in dB (typical range: -60 – 0 dB). */
	void setThreshold(float dB);

	/** Compression ratio (e.g. 4.0 = 4:1).  Must be >= 1.0. */
	void setRatio(float ratio);

	/** If true, use a soft knee (smooth transition around threshold). */
	void setKnee(bool soft);

	/**
	 * Enable or disable lookahead.
	 *
	 * When enabled the envelope detector reads the current (pre-delay) sample
	 * so gain reduction is fully active before the delayed output arrives —
	 * zero-attack transient control.
	 *
	 * When disabled the envelope reads the delayed (output) sample, giving
	 * classic feed-forward behaviour.
	 *
	 * The 2 ms output delay is always present to keep bands time-aligned.
	 */
	void setLookaheadEnabled(bool enabled);

	/** Makeup gain in dB applied after compression. */
	void setMakeupGain(float dB);

	// -----------------------------------------------------------------------
	//  Delay reporting
	// -----------------------------------------------------------------------

	/** @return the fixed 2 ms lookahead delay in samples (always > 0). */
	[[nodiscard]] int getLookaheadDelaySamples() const noexcept;

	/** @return true if lookahead mode is enabled (envelope reads pre-delay). */
	[[nodiscard]] bool hasLookahead() const noexcept;

	// -----------------------------------------------------------------------
	//  Getters  (return the target value, not the current ramped value)
	// -----------------------------------------------------------------------

	[[nodiscard]] float getAttack() const noexcept;
	[[nodiscard]] float getRelease() const noexcept;
	[[nodiscard]] float getThreshold() const noexcept;
	[[nodiscard]] float getRatio() const noexcept;
	[[nodiscard]] bool getKnee() const noexcept;
	[[nodiscard]] bool getLookaheadEnabled() const noexcept;
	[[nodiscard]] float getMakeupGain() const noexcept;

  private:
	// -----------------------------------------------------------------------
	//  Per-channel state
	// -----------------------------------------------------------------------
	struct ChannelState
	{
		// Smoothed envelope (linear, the IIR lowpass of the squared signal)
		float envelopeSq = 0.0f;

		// Smoothed gain reduction in dB (after ballistics)
		float gainReductionDb = 0.0f;
	};
	std::vector<ChannelState> channels;

	// -----------------------------------------------------------------------
	//  Parameters  (SmoothedValue for click-free ramping)
	// -----------------------------------------------------------------------
	juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> attackMs{10.0f};
	juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> releaseMs{100.0f};
	juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> thresholdDb{-24.0f};
	juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> ratio{4.0f};
	juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> makeupGainDb{0.0f};
	bool softKnee = true;
	bool lookaheadEnabled = false;

	// -----------------------------------------------------------------------
	//  Processing state
	// -----------------------------------------------------------------------
	double sampleRate = 44100.0;
	int blockSize = 512;
	int numChannels = 0;

	// Cached time-constant coefficients (recalculated when attack/release ramp)
	float attackCoeff = 0.0f;
	float releaseCoeff = 0.0f;

	// Envelope detector time constant (fixed at ~5 ms for VCA-style response)
	float envelopeAlpha = 0.0f;
	bool envelopeAlphaDirty = true;

	// Delay line — always runs at 2 ms to keep all compressors time-aligned.
	//   writePos:    where the current sample is written
	//   delayed read: always from (writePos - delayLength) mod delayLength
	struct DelayState
	{
		std::vector<float> buffer;
		int writePos = 0;
	};
	std::vector<DelayState> delayStates;
	int delayLengthSamples = 0;

	static constexpr float fixedDelayLengthMs = 2.0f;
	static constexpr double fixedSmoothingSeconds = 0.03;

	// -----------------------------------------------------------------------
	//  Helpers
	// -----------------------------------------------------------------------
	void updateEnvelopeAlpha();
	float computeGainReduction(float envelopeDb, float threshDb, float currentRatio) const;
};