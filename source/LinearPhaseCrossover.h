#pragma once

#include <juce_dsp/juce_dsp.h>
#include <vector>

/**
 * Splits an audio signal into 3 bands (low, mid, high) using linear phase
 * crossover filtering.
 *
 * The crossover uses windowed-sinc FIR lowpass filters designed with a Kaiser
 * window, convolved in the frequency domain via juce::dsp::Convolution.
 * Because the FIR filters have symmetric (linear phase) impulse responses,
 * they introduce a constant group delay of (N-1)/2 samples across all
 * frequencies.  The mid and high bands are derived by subtraction, so all
 * three bands share the same group delay and are phase-aligned with each
 * other.
 *
 * Call prepare() before first use, and whenever the sample rate or
 * maximum block size changes.
 */
class LinearPhaseCrossover
{
  public:
	LinearPhaseCrossover() = default;

	/** Initialise internal buffers. Must be called before split(). */
	void prepare(double sampleRate, int blockSize);

	/** Set the low crossover frequency in Hz (must be below the high crossover).
	 */
	void setLowCrossover(float frequencyHz);

	/** Set the high crossover frequency in Hz (must be above the low crossover).
	 */
	void setHighCrossover(float frequencyHz);

	/** @return the current low crossover frequency in Hz. */
	[[nodiscard]] float getLowCrossover() const noexcept;

	/** @return the current high crossover frequency in Hz. */
	[[nodiscard]] float getHighCrossover() const noexcept;

	/** @return the current sample rate in Hz. */
	[[nodiscard]] double getSampleRate() const noexcept;

	/**
	 * Split @p buffer into three bands.  After this call the three bands
	 * are accessible via getLowBand(), getMidBand() and getHighBand().
	 * The number of channels and samples read from @p buffer must match
	 * the values passed to prepare().
	 */
	void split(juce::AudioBuffer<float>& buffer);

	/** Pointer to the low-band output for @p channel (valid after split()). */
	[[nodiscard]] const float* getLowBand(int channel) const;

	/** Pointer to the mid-band output for @p channel (valid after split()). */
	[[nodiscard]] const float* getMidBand(int channel) const;

	/** Pointer to the high-band output for @p channel (valid after split()). */
	[[nodiscard]] const float* getHighBand(int channel) const;

	/**
	 * Returns true — a linear phase FIR filter always introduces a group
	 * delay of (N-1)/2 samples.  Use getGroupDelaySamples() to query the
	 * exact value.
	 */
	[[nodiscard]] bool hasDelay() const noexcept;

	/**
	 * @return the group delay introduced by the FIR filters, in samples.
	 * This is (N-1)/2 where N is the number of filter taps.
	 */
	[[nodiscard]] int getGroupDelaySamples() const noexcept;

  private:
	/** Design a windowed-sinc lowpass FIR at the given cutoff (Hz). */
	static std::vector<float> designLowpass(float cutoffHz, double sampleRate);

	/** Modified Bessel function I0 (full range). */
	static float besselI0(float x);

	static bool floatMatch(float a, float b);

	static constexpr float floatEpsilon = 0.00001f;

	double sampleRate = 44100.0;
	int blockSize = 512;

	// Currently-active crossover frequencies used for filter design.
	// Updated from the smoothers inside split() when a ramp is in progress.
	float lowCrossoverHz = 200.0f;
	float highCrossoverHz = 2000.0f;

	// Smoothed crossover frequency targets — setLowCrossover() /
	// setHighCrossover() set the *target* and the smoother ramps to it over ~50
	// ms so that filter redesigns happen gradually on a per-block basis.
	juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative> smoothedLowCrossover{lowCrossoverHz};
	juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative> smoothedHighCrossover{highCrossoverHz};

	// FIR coefficients (kept for group-delay calculation and IR loading)
	std::vector<float> lowCoeffs;  // lowpass @ lowCrossover
	std::vector<float> highCoeffs; // lowpass @ highCrossover

	// JUCE DSP convolution engines — replace manual time-domain convolution
	// with FFT-based partitioned convolution for O(N log N) performance.
	juce::dsp::Convolution lowConvolution;
	juce::dsp::Convolution highConvolution;

	// Pre-allocated output buffers for convolution results  [channel][sample]
	juce::AudioBuffer<float> lowConvOutput;
	juce::AudioBuffer<float> highConvOutput;

	// Delay history for the original signal — used to align the delayed
	// input with the convolution output for band subtraction.
	// Stored as a circular buffer per channel.
	struct DelayState
	{
		std::vector<float> buffer;
		int writePos = 0;
	};
	std::vector<DelayState> delayStates;
	int delayLength = 0; // = group delay = (K-1)/2

	// Output band buffers  [channel][sample]
	std::vector<std::vector<float>> lowBand;
	std::vector<std::vector<float>> midBand;
	std::vector<std::vector<float>> highBand;

	int numChannels = 0;

	void designFilters();
	void ensureBuffers(int channels);
	void prepareConvolutions();
	void loadImpulseResponses();
};