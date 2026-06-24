#include "Limiter.h"

#include <algorithm>
#include <cmath>
#include <vector>

//==============================================================================
Limiter::Limiter() = default;

//==============================================================================
void Limiter::prepare(double sr, int bs, int nc)
{
	const bool reallocate = oversampler == nullptr || !juce::approximatelyEqual(sr, sampleRate) ||
							!juce::approximatelyEqual(blockSize, bs) || numChannels != nc;

	sampleRate = sr;
	blockSize = bs;
	numChannels = nc;

	if (reallocate)
	{
		// Initialise 16x oversampler using FIR equiripple filter for linear phase (transparent)
		oversampler = std::make_unique<juce::dsp::Oversampling<float>>(
			numChannels, oversampleStages, juce::dsp::Oversampling<float>::FilterType::filterHalfBandFIREquiripple);

		oversampler->initProcessing(static_cast<size_t>(blockSize));

		// Initialise lookahead delay (1ms at current sample rate)
		const int lookaheadSamples = static_cast<int>(std::round(lookaheadMs * sampleRate / 1000.0));
		lookaheadDelay.prepare(
			{sampleRate, static_cast<juce::uint32>(blockSize), static_cast<juce::uint32>(numChannels)});
		lookaheadDelay.setMaximumDelayInSamples(static_cast<int>(lookaheadSamples) + 1);
		lookaheadDelay.setDelay(static_cast<int>(lookaheadSamples));

		// Initialise per-channel state
		channelStates.resize(static_cast<size_t>(numChannels));

		// Prepare smoothed parameters
		ceilingDb.reset(sampleRate * oversampleFactor, 0.05); // 50ms smoothing for ceiling changes
		ceilingDb.setCurrentAndTargetValue(ceilingDb.getTargetValue());
	}
	else
	{
		reset();
	}
}

//==============================================================================
void Limiter::process(juce::AudioBuffer<float>& buffer)
{
	const int numSamples = buffer.getNumSamples();
	const int numCh = buffer.getNumChannels();

	if (numSamples == 0 || numCh == 0)
		return;

	// Resize channelStates if needed (e.g. after reset() cleared it)
	if (static_cast<int>(channelStates.size()) != numCh)
		channelStates.resize(static_cast<size_t>(numCh));

	// --- 1. Apply lookahead delay to input ---
	// We delay the input so the envelope detector can "look ahead"
	juce::dsp::AudioBlock<float> inputBlock(buffer);
	juce::dsp::ProcessContextReplacing<float> delayContext(inputBlock);
	lookaheadDelay.process(delayContext);

	// --- 2. Upsample to 16x ---
	juce::dsp::AudioBlock<float> oversampledBlock = oversampler->processSamplesUp(inputBlock);
	const auto oversampledNumSamples = oversampledBlock.getNumSamples();

	// --- 3. Get ramped ceiling values ---
	std::vector<float> ceilingLinear(oversampledNumSamples);
	for (size_t n = 0; n < oversampledNumSamples; ++n)
	{
		ceilingLinear[n] = juce::Decibels::decibelsToGain(ceilingDb.getNextValue());
	}

	// --- 4. Process at 16x sample rate ---
	// Detect peaks and apply gain reduction on the oversampled signal
	for (size_t ch = 0; ch < static_cast<size_t>(numCh); ++ch)
	{
		auto* channelData = oversampledBlock.getChannelPointer(ch);
		auto& state = channelStates[ch];

		for (size_t n = 0; n < oversampledNumSamples; ++n)
		{
			const float sample = channelData[n];
			const float absSample = std::abs(sample);

			// Update peak level (fast attack)
			if (absSample > state.peakLevel)
			{
				// Fast attack: immediate response to peaks
				state.peakLevel = absSample;
			}
			else
			{
				// Slow release: let peak decay naturally
				const float releaseTimeMs = computeReleaseTime(state.gainReductionDb);
				const float releaseCoeff = 1.0f - std::exp(-1.0f / (releaseTimeMs * static_cast<float>(sampleRate) *
																	static_cast<float>(oversampleFactor) / 1000.0f));
				state.peakLevel *= (1.0f - releaseCoeff);
				state.peakLevel = std::max(state.peakLevel, absSample);
			}

			// Compute required gain reduction
			float targetGainReductionDb = 0.0f;
			if (state.peakLevel > ceilingLinear[n])
			{
				targetGainReductionDb = juce::Decibels::gainToDecibels(ceilingLinear[n] / state.peakLevel);
			}

			// Fast attack, smoothed release envelope for gain reduction.
			// Attack is instantaneous to ensure brick-wall limiting (catching transients).
			// Release is smoothed to prevent zipper noise when the signal falls below the ceiling.
			if (targetGainReductionDb < state.gainReductionDb)
			{
				// More reduction needed → instant attack (enforces the brick-wall ceiling)
				state.gainReductionDb = targetGainReductionDb;
			}
			else
			{
				// Less reduction needed → smoothed release (prevents zipper noise)
				const float releaseTimeMs = computeReleaseTime(state.gainReductionDb);
				const float releaseCoeff = 1.0f - std::exp(-1.0f / (releaseTimeMs * static_cast<float>(sampleRate) *
																	static_cast<float>(oversampleFactor) / 1000.0f));
				state.gainReductionDb += (targetGainReductionDb - state.gainReductionDb) * releaseCoeff;
			}

			// Apply gain reduction
			const float gainLinear = juce::Decibels::decibelsToGain(state.gainReductionDb);
			channelData[n] = sample * gainLinear;
		}
	}

	// --- 5. Downsample back to original rate ---
	oversampler->processSamplesDown(inputBlock);
}

//==============================================================================
void Limiter::reset()
{
	if (oversampler)
		oversampler->reset();

	lookaheadDelay.reset();

	for (auto& ch : channelStates)
		ch = ChannelState{};

	ceilingDb.reset(sampleRate * oversampleFactor, 0.05);
	ceilingDb.setCurrentAndTargetValue(ceilingDb.getTargetValue());
}

//==============================================================================
void Limiter::setCeiling(float dB)
{
	ceilingDb.setTargetValue(juce::jlimit(-maxGainReductionDb, 0.0f, dB));
}

//==============================================================================
float Limiter::getCeiling() const noexcept
{
	return ceilingDb.getTargetValue();
}

//==============================================================================
int Limiter::getLatencySamples() const noexcept
{
	// Latency = lookahead delay + oversampling latency
	const int lookaheadSamples = static_cast<int>(std::round(lookaheadMs * sampleRate / 1000.0));
	const int oversamplingLatency = oversampler ? static_cast<int>(oversampler->getLatencyInSamples()) : 0;
	return lookaheadSamples + oversamplingLatency;
}

//==============================================================================
//  Gain reduction monitoring
//==============================================================================

float Limiter::getGainReductionDb(int channel) const noexcept
{
	if (channel >= 0 && channel < numChannels)
		return channelStates[static_cast<size_t>(channel)].gainReductionDb;
	return 0.0f;
}

float Limiter::getMaxGainReductionDb() const noexcept
{
	float maxGR = 0.0f;
	for (const auto& ch : channelStates)
	{
		maxGR = std::min(maxGR, ch.gainReductionDb);
	}
	return maxGR;
}

//==============================================================================
float Limiter::computeReleaseTime(float gainReductionDb) const
{
	// Dynamic release: faster release for heavier limiting, slower for lighter limiting
	// This prevents pumping while maintaining transparency
	const float normalisedGR = juce::jlimit(0.0f, 1.0f, std::abs(gainReductionDb) / maxGainReductionDb);

	// Interpolate between min and max release times
	// Heavy limiting (high GR) -> faster release
	// Light limiting (low GR) -> slower release
	return juce::jmap(normalisedGR, minReleaseMs, maxReleaseMs);
}