#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

  /**
   * A flexible waveshaping saturation processor that operates on multi-channel
   * audio buffers.
   *
   * Controls:
   *   - Drive     (0.0 – 100.0): Saturation amount. Higher values push the signal
   *                              harder into the waveshaper.  The value is stored
   *                              as-is and mapped internally to a 0.0 – 1.0 range
   *                              when processing.
   *   - Even/Odd  (-100 – 100): Crossfades between odd-harmonic (symmetric) and
   *                              even-harmonic (asymmetric) distortion.  -100
   *                              produces pure odd harmonics, +100 produces rich
   *                              even harmonics.  The exact character depends on
   *                              the mode (see Heavy mode toggle).
   *   - Heavy mode toggle:       Mild mode uses a single-stage tanh-based soft
   *                              clipper with drive range 1-4x.  Heavy mode uses
   *                              two cascaded stages of cubic waveshaping with DC
   *                              offset for asymmetric clipping, with drive range
   *                              1-8x per stage for a distinctly different,
   *                              more aggressive character.
   *   - Wet/Dry  (-100 – 100):   Mix between the dry input and the processed
   *                              signal.  -100 is fully dry, 0 is an equal mix,
   *                              and +100 is fully wet (saturation output only).
   *                              The value is stored as-is and mapped internally
   *                              to a 0.0 – 1.0 range during processing.
   *
   * Call prepare() before first use, and whenever the sample rate or block size
   * changes.  The process() method works in-place on a juce::AudioBuffer<float>
   * and handles any number of channels.
   */
class Saturation
{
  public:
	Saturation() = default;

	/** Initialise internal state. Must be called before process(). */
	void prepare(double sampleRate, int blockSize);

	/**
	 * Process one block of audio in-place.
	 *
	 * @param buffer  The audio buffer to process.  Works with any number of
	 *                channels.
	 */
	void process(juce::AudioBuffer<float>& buffer);

	// -----------------------------------------------------------------------
	//  Parameter setters
	// -----------------------------------------------------------------------

	/** Saturation amount (0.0 – 100.0).  The value is stored as-is and mapped
	 *  internally to a 0.0 – 1.0 range during processing. */
	void setDrive(float drive);

	/**
	 * Even/odd harmonics balance.
	 *
	 *   -100.0f  →  pure odd harmonics (symmetric transfer function)
	 *     +0.0f  →  balanced mix
	 *   +100.0f  →  pure even harmonics (asymmetric transfer function)
	 */
	void setEvenOddBalance(float balance);

	/** Toggle between mild (false) and heavy (true) saturation mode. */
	void setHeavyMode(bool heavy);

	/** Wet/dry mix (-100.0 = fully dry, 0.0 = equal mix, +100.0 = fully wet).
	 *  The value is stored as-is and mapped internally to a 0.0 – 1.0 range
	 *  during processing. */
	void setMix(float mix);

	// -----------------------------------------------------------------------
	//  Getters
	// -----------------------------------------------------------------------

	[[nodiscard]] float getDrive() noexcept;
	[[nodiscard]] float getEvenOddBalance() noexcept;
	[[nodiscard]] bool getHeavyMode() const noexcept;
	[[nodiscard]] float getMix() noexcept;

  private:
	// -----------------------------------------------------------------------
	//  Parameters (smoothed)
	// -----------------------------------------------------------------------
	juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> drive = {50};
	juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> evenOddBalance = {0};
	bool heavyMode = false;
	juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> mix = {0};

	// -----------------------------------------------------------------------
	//  Processing state
	// -----------------------------------------------------------------------
	double sampleRate = 44100.0;
	int blockSize = 512;

	// -----------------------------------------------------------------------
	//  Sample-level processing
	// -----------------------------------------------------------------------
	/** Process a single sample through the saturation waveshaper. */
	float processSample(float x);
};
