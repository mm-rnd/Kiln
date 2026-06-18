#include "Gain.h"
#include <algorithm>
#include <cmath>

// ---------------------------------------------------------------------------
//  Preparation
// ---------------------------------------------------------------------------

void Gain::prepare(double sampleRate)
{
	// Reset and configure smoothed parameters
	gainDb.reset(sampleRate, 0.05); // 50 ms smoothing time
	gainDb.setCurrentAndTargetValue(gainDb.getTargetValue());
}

// ---------------------------------------------------------------------------
//  Parameter setters
// ---------------------------------------------------------------------------

void Gain::setGain(float g)
{
	// Set target for smoothed gain (-24.0 – +24.0 dB).
	gainDb.setTargetValue(std::clamp(g, -24.0f, 24.0f));
}

// ---------------------------------------------------------------------------
//  Parameter getters
// ---------------------------------------------------------------------------

float Gain::getGain() const noexcept
{
	return gainDb.getTargetValue();
}

// ---------------------------------------------------------------------------
//  Process block
// ---------------------------------------------------------------------------

void Gain::process(juce::AudioBuffer<float>& buffer)
{
	const int numChannels = buffer.getNumChannels();
	const int numSamples = buffer.getNumSamples();

	if (numChannels == 0 || numSamples == 0)
		return;

	// Get the smoothed gain values for this block
	std::vector<float> gains(static_cast<size_t>(numSamples));
	for (size_t n = 0; n < static_cast<size_t>(numSamples); ++n)
	{
		const float currentGainDb = gainDb.getNextValue();
		const float linearGain = juce::Decibels::decibelsToGain(currentGainDb);
		gains[n] = linearGain;
	}

	for (int ch = 0; ch < numChannels; ++ch)
	{
		auto* channelData = buffer.getWritePointer(ch);
		for (int n = 0; n < numSamples; ++n)
			channelData[n] *= gains[static_cast<size_t>(n)];
	}
}