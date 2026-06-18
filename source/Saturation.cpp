#include "Saturation.h"
#include <algorithm>
#include <cmath>
#include <juce_dsp/juce_dsp.h>

// ---------------------------------------------------------------------------
//  Preparation
// ---------------------------------------------------------------------------

void Saturation::prepare(double sr, int bs)
{
	sampleRate = sr;
	blockSize = bs;

	// Reset and configure smoothed parameters
	drive.reset(sr, 0.05);   // 50 ms smoothing time
	drive.setCurrentAndTargetValue(drive.getTargetValue());

	evenOddBalance.reset(sr, 0.05);
	evenOddBalance.setCurrentAndTargetValue(evenOddBalance.getTargetValue());

	mix.reset(sr, 0.05);
	mix.setCurrentAndTargetValue(mix.getTargetValue());
}

// ---------------------------------------------------------------------------
//  Parameter setters
// ---------------------------------------------------------------------------

void Saturation::setDrive(float d)
{
	// Set target for smoothed drive (0.0 – 100.0).
	drive.setTargetValue(std::clamp(d, 0.0f, 100.0f));
}

void Saturation::setEvenOddBalance(float balance)
{
	// Set target for smoothed even/odd balance (-100.0 – 100.0).
	evenOddBalance.setTargetValue(std::clamp(balance, -100.0f, 100.0f));
}

void Saturation::setHeavyMode(bool heavy)
{
	heavyMode = heavy;
}

void Saturation::setMix(float m)
{
	// Set target for smoothed mix (-100.0 – 100.0).
	mix.setTargetValue(std::clamp(m, -100.0f, 100.0f));
}

// ---------------------------------------------------------------------------
//  Parameter Getters
// ---------------------------------------------------------------------------

float Saturation::getDrive() noexcept
{
	return drive.getTargetValue();
}

float Saturation::getEvenOddBalance() noexcept
{
	return evenOddBalance.getTargetValue();
}

bool Saturation::getHeavyMode() const noexcept
{
	return heavyMode;
}

float Saturation::getMix() noexcept
{
	return mix.getTargetValue();
}

// ---------------------------------------------------------------------------
//  Sample-level processing
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  Waveshaping functions
// ---------------------------------------------------------------------------

/** Mild mode: tanh-based soft clipper */
static float mildShaper(float x, float balance)
{
	// --- Odd-harmonic waveshaper (symmetric) ---
	//   tanh is an odd-symmetric function → pure odd harmonics
	const float oddOut = std::tanh(x);

	// --- Even-harmonic waveshaper (asymmetric) ---
	//   Adding an x * |x| term breaks symmetry, introducing even harmonics.
	constexpr float evenAsym = 0.8f;
	const float evenOut = std::tanh(x) + evenAsym * std::max(0.0f, x); // half-wave adds strong 2nd harmonic

	// Crossfade between odd and even based on balance parameter.
	//  -100 → pure odd     (t = 0.0)
	//     0 → balanced     (t = 0.5)
	//  +100 → pure even    (t = 1.0)
	const float t = (balance + 100.0f) * (1.0f / 200.0f);
	return oddOut * (1.0f - t) + evenOut * t;
}

/** Heavy mode: cubic waveshaper with DC offset for asymmetric clipping */
static float heavyShaper(float x, float balance)
{
	// Add DC offset to create asymmetric clipping (richer even harmonics)
	constexpr float dcOffset = 0.3f;
	const float xOffset = x + dcOffset;

	// Cubic waveshaper: x - x³/3 (normalized)
	// This creates a distinctly different character from tanh
	const float cubic = xOffset - (xOffset * xOffset * xOffset) / 3.0f;

	// Remove DC offset after waveshaping
	const float shaped = cubic - dcOffset;

	// For heavy mode, blend between symmetric cubic and asymmetric based on balance
	// -100 → symmetric cubic (pure odd harmonics)
	// +100 → asymmetric with DC offset (rich even harmonics)
	const float t = (balance + 100.0f) * (1.0f / 200.0f);
	const float symmetricCubic = x - (x * x * x) / 3.0f;
	return symmetricCubic * (1.0f - t) + shaped * t;
}

float Saturation::processSample(float x)
{
	// Get smoothed parameter values for this sample.
	const float currentDrive = drive.getNextValue();
	const float currentEvenOdd = evenOddBalance.getNextValue();
	const float currentMix = mix.getNextValue();

	// Map drive to pre-gain based on mode.
	//   Mild:  pre-gain range 1.0 – 4.0
	//   Heavy: pre-gain range 1.0 – 8.0
	const float maxDrive = heavyMode ? 8.0f : 4.0f;
	const float driveGain = 1.0f + (currentDrive * 0.01f) * (maxDrive - 1.0f);

	const float xDriven = x * driveGain;

	float shaped;
	if (heavyMode)
	{
		// Heavy mode: two cascaded stages of cubic waveshaper with DC offset
		const float stage1 = heavyShaper(xDriven, currentEvenOdd);
		const float stage2 = heavyShaper(stage1 * 1.2f, currentEvenOdd);  // Slight gain increase for second stage
		shaped = stage2;
	}
	else
	{
		// Mild mode: single-stage tanh-based waveshaper
		shaped = mildShaper(xDriven, currentEvenOdd);
	}

	// Wet/dry mix: map stored -100..100 to 0.0..1.0 at usage time.
	const float mix01 = (currentMix + 100.0f) * 0.005f;
	return x * (1.0f - mix01) + shaped * mix01;
}

// ---------------------------------------------------------------------------
//  Process block
// ---------------------------------------------------------------------------

void Saturation::process(juce::AudioBuffer<float>& buffer)
{
	const int numChannels = buffer.getNumChannels();
	const int numSamples = buffer.getNumSamples();

	if (numChannels == 0 || numSamples == 0)
		return;

	for (int ch = 0; ch < numChannels; ++ch)
	{
		auto* channelData = buffer.getWritePointer(ch);
		for (int n = 0; n < numSamples; ++n)
			channelData[n] = processSample(channelData[n]);
	}
}
