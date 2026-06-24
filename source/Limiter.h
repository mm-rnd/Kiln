#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

/**
 * A transparent brick-wall limiter with 16x oversampling and lookahead.
 *
 * Designed to work on summed (mixed) audio to prevent clipping while
 * maintaining maximum transparency. Uses:
 *
 *   - 16x oversampling to catch inter-sample peaks
 *   - 1ms lookahead to anticipate transients
 *   - Dynamic release to prevent pumping
 *   - Smoothed ceiling parameter for click-free parameter changes
 *
 * The limiter uses a fast-attack, dynamically-adjusted-release envelope
 * follower that ensures transparent limiting even at high reduction ratios.
 */
class Limiter
{
  public:
	Limiter();

	/** Initialise internal state. Must be called before process(). */
	void prepare(double sr, int bs, int nc);

	/**
	 * Process one block of audio in-place.
	 *
	 * @param buffer  The audio buffer to process. Works with any number of
	 *                channels.
	 */
	void process(juce::AudioBuffer<float>& buffer);

	/** Reset all internal state. */
	void reset();

	/** Set the ceiling level in dB (typical range: -24 to 0 dB). */
	void setCeiling(float dB);

	/** Get the current target ceiling in dB. */
	[[nodiscard]] float getCeiling() const noexcept;

	/** @return total latency in samples (lookahead + oversampling). */
	[[nodiscard]] int getLatencySamples() const noexcept;

	// -----------------------------------------------------------------------
	//  Gain reduction monitoring
	// -----------------------------------------------------------------------

	/** Get the current gain reduction in dB for a specific channel. */
	[[nodiscard]] float getGainReductionDb(int channel) const noexcept;

	/** Get the maximum gain reduction across all channels in dB. */
	[[nodiscard]] float getMaxGainReductionDb() const noexcept;

  private:
	// -----------------------------------------------------------------------
	//  Parameters (smoothed for click-free changes)
	// -----------------------------------------------------------------------
	juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> ceilingDb{0.0f};

	// -----------------------------------------------------------------------
	//  16x Oversampling
	// -----------------------------------------------------------------------
	std::unique_ptr<juce::dsp::Oversampling<float>> oversampler;
	static constexpr int oversampleFactor = 16;
	static constexpr int oversampleStages = 4; // 2^4 = 16

	// -----------------------------------------------------------------------
	//  Lookahead Delay
	// -----------------------------------------------------------------------
	juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> lookaheadDelay;
	static constexpr float lookaheadMs = 1.0f; // 1ms lookahead

	// -----------------------------------------------------------------------
	//  Processing State
	// -----------------------------------------------------------------------
	double sampleRate = 44100.0;
	int blockSize = 0;
	int numChannels = 0;

	// Per-channel envelope state
	struct ChannelState
	{
		float peakLevel = 0.0f;		  // Current peak level (linear)
		float gainReductionDb = 0.0f; // Smoothed gain reduction (dB)
	};
	std::vector<ChannelState> channelStates;

	// -----------------------------------------------------------------------
	//  Dynamic Release Parameters
	// -----------------------------------------------------------------------
	static constexpr float minReleaseMs = 50.0f;  // Fast release for heavy limiting
	static constexpr float maxReleaseMs = 500.0f; // Slow release to prevent pumping
	static constexpr float maxGainReductionDb = 24.0f;

	// -----------------------------------------------------------------------
	//  Helpers
	// -----------------------------------------------------------------------
	[[nodiscard]] float computeReleaseTime(float gainReductionDb) const;
};