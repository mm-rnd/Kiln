#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

/**
 * A simple gain processor with parameter smoothing.
 *
 * Converts dB values to linear gain internally and applies smoothed
 * parameter changes to avoid clicks and zipper noise.
 */
class Gain
{
  public:
	Gain() = default;

	/** Initialise internal state. Must be called before process(). */
	void prepare(double sampleRate);

	/**
	 * Process one block of audio in-place.
	 *
	 * @param buffer  The audio buffer to process.  Works with any number of
	 *                channels.
	 */
	void process(juce::AudioBuffer<float>& buffer);

	/** Set gain in dB (-24.0 to +24.0). */
	void setGain(float gainDb);

	/** Get current target gain in dB. */
	[[nodiscard]] float getGain() const noexcept;

  private:
	// -----------------------------------------------------------------------
	//  Parameters (smoothed)
	// -----------------------------------------------------------------------
	juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> gainDb = {0.0f};
};