#include "Compressor.h"
#include <algorithm>
#include <cmath>
#include <cstring>

// ---------------------------------------------------------------------------
//  Preparation
// ---------------------------------------------------------------------------

void Compressor::prepare(double sr, int bs, int numCh)
{
	sampleRate = sr;
	blockSize = bs;
	numChannels = numCh;

	// Resize per-channel state
	channels.resize(static_cast<size_t>(numCh));

	// Initialise SmoothedValue ramps (30 ms ramp)
	auto resetSmoother = [&](auto& smoother)
	{
		const float target = smoother.getTargetValue();
		smoother.reset(sampleRate, fixedSmoothingSeconds);
		smoother.setCurrentAndTargetValue(target);
	};
	resetSmoother(attackMs);
	resetSmoother(releaseMs);
	resetSmoother(thresholdDb);
	resetSmoother(ratio);
	resetSmoother(makeupGainDb);

	// Mark envelope alpha as dirty — it depends on sample rate
	envelopeAlphaDirty = true;

	// Allocate delay line — always at the fixed 2 ms length.
	// This ensures all compressor instances have identical latency regardless
	// of per-band lookahead state, keeping crossover bands phase-aligned.
	delayLengthSamples = static_cast<int>(std::ceil(fixedDelayLengthMs * 0.001f * static_cast<float>(sampleRate)));

	delayStates.resize(static_cast<size_t>(numCh));
	for (int ch = 0; ch < numCh; ++ch)
	{
		auto& ds = delayStates[static_cast<size_t>(ch)];
		ds.buffer.assign(static_cast<size_t>(delayLengthSamples), 0.0f);
		ds.writePos = 0;
	}
}

// ---------------------------------------------------------------------------
//  Parameter setters  (set SmoothedValue targets)
// ---------------------------------------------------------------------------

void Compressor::setAttack(float ms)
{
	attackMs.setTargetValue(std::max(0.01f, ms));
}

void Compressor::setRelease(float ms)
{
	releaseMs.setTargetValue(std::max(1.0f, ms));
}

void Compressor::setThreshold(float dB)
{
	thresholdDb.setTargetValue(dB);
}

void Compressor::setRatio(float r)
{
	ratio.setTargetValue(std::max(1.0f, r));
}

void Compressor::setKnee(bool soft)
{
	softKnee = soft;
}

void Compressor::setLookaheadEnabled(bool enabled)
{
	lookaheadEnabled = enabled;
}

void Compressor::setMakeupGain(float dB)
{
	makeupGainDb.setTargetValue(dB);
}

// ---------------------------------------------------------------------------
//  Getters  (return target values)
// ---------------------------------------------------------------------------

float Compressor::getAttack() const noexcept { return attackMs.getTargetValue(); }
float Compressor::getRelease() const noexcept { return releaseMs.getTargetValue(); }
float Compressor::getThreshold() const noexcept { return thresholdDb.getTargetValue(); }
float Compressor::getRatio() const noexcept { return ratio.getTargetValue(); }
bool Compressor::getKnee() const noexcept { return softKnee; }
bool Compressor::getLookaheadEnabled() const noexcept { return lookaheadEnabled; }
float Compressor::getMakeupGain() const noexcept { return makeupGainDb.getTargetValue(); }

int Compressor::getLookaheadDelaySamples() const noexcept
{
	return delayLengthSamples;
}

bool Compressor::hasLookahead() const noexcept
{
	return lookaheadEnabled;
}

// ---------------------------------------------------------------------------
//  Coefficient updates
// ---------------------------------------------------------------------------

void Compressor::updateEnvelopeAlpha()
{
	// Envelope detector: one-pole lowpass at ~5 ms for RMS-style tracking
	const float tau = 0.005f; // 5 ms
	envelopeAlpha = std::exp(-1.0f / (tau * static_cast<float>(sampleRate)));
	envelopeAlphaDirty = false;
}

// ---------------------------------------------------------------------------
//  Gain computer
// ---------------------------------------------------------------------------

float Compressor::computeGainReduction(float envelopeDb, float threshDb, float currentRatio) const
{
	if (envelopeDb <= threshDb)
		return 0.0f;

	const float overshootDb = envelopeDb - threshDb;

	if (softKnee)
	{
		constexpr float kneeWidth = 6.0f;
		const float kneeHalf = kneeWidth * 0.5f;
		const float lowerBound = threshDb - kneeHalf;
		const float upperBound = threshDb + kneeHalf;

		if (envelopeDb <= lowerBound)
			return 0.0f;

		if (envelopeDb >= upperBound)
			return overshootDb * (1.0f - 1.0f / currentRatio);

		const float x = (envelopeDb - lowerBound) / kneeWidth;
		const float curve = (1.0f - 1.0f / currentRatio) * 3.0f * x * x;
		return curve * overshootDb;
	}

	return overshootDb * (1.0f - 1.0f / currentRatio);
}

// ---------------------------------------------------------------------------
//  Process
// ---------------------------------------------------------------------------

void Compressor::process(const float* const* inputChannels, float* const* outputChannels, int numSamples)
{
	if (numChannels == 0 || numSamples == 0)
		return;

	// Update envelope alpha if needed (depends on sample rate only)
	if (envelopeAlphaDirty)
		updateEnvelopeAlpha();

	const int delayLen = delayLengthSamples;

	for (int ch = 0; ch < numChannels; ++ch)
	{
		const float* in = inputChannels[ch];
		float* out = outputChannels[ch];
		auto& state = channels[static_cast<size_t>(ch)];
		auto& ds = delayStates[static_cast<size_t>(ch)];

		for (int n = 0; n < numSamples; ++n)
		{
			// Advance all SmoothedValue ramps one sample
			const float currentAttackMs = attackMs.getNextValue();
			const float currentReleaseMs = releaseMs.getNextValue();
			const float currentThresholdDb = thresholdDb.getNextValue();
			const float currentRatio = ratio.getNextValue();
			const float currentMakeupGainDb = makeupGainDb.getNextValue();

			// Recompute ballistics coefficients from the current ramped values
			{
				const float attackTau = currentAttackMs * 0.001f;
				const float releaseTau = currentReleaseMs * 0.001f;
				attackCoeff = std::exp(-1.0f / (attackTau * static_cast<float>(sampleRate)));
				releaseCoeff = std::exp(-1.0f / (releaseTau * static_cast<float>(sampleRate)));
			}

			const float makeupLinear = juce::Decibels::decibelsToGain(currentMakeupGainDb);

			// --- Delay line (always active, fixed length) ------------------
			// The delay line ensures all compressor instances have identical
			// latency, keeping bands phase-aligned when summed.
			const float currentSample = in[n];

			// Read the delayed sample (oldest in the buffer)
			const int readPos = (ds.writePos + 1) % delayLen;
			const float delayedSample = ds.buffer[static_cast<size_t>(readPos)];

			// Write the current sample into the buffer
			ds.buffer[static_cast<size_t>(ds.writePos)] = currentSample;
			ds.writePos = (ds.writePos + 1) % delayLen;

			// --- Envelope source -------------------------------------------
			// Lookahead ON:  envelope reads the current (pre-delay) sample
			//                → gain reduction precedes the delayed output
			// Lookahead OFF: envelope reads the delayed (output) sample
			//                → classic feed-forward behaviour
			const float envSample = lookaheadEnabled ? currentSample : delayedSample;

			// --- Envelope detection (RMS-style) ---------------------------
			const float inputSq = envSample * envSample;
			state.envelopeSq += (1.0f - envelopeAlpha) * (inputSq - state.envelopeSq);

			const float envelopeLin = std::sqrt(state.envelopeSq + 1e-12f);
			const float envelopeDb = juce::Decibels::gainToDecibels(envelopeLin);

			// --- Gain computer ---------------------------------------------
			const float targetGR = computeGainReduction(envelopeDb, currentThresholdDb, currentRatio);

			// --- Ballistics (attack/release smoothing on GR in dB) ---------
			if (targetGR > state.gainReductionDb)
			{
				state.gainReductionDb += (1.0f - attackCoeff) * (targetGR - state.gainReductionDb);
			}
			else
			{
				state.gainReductionDb += (1.0f - releaseCoeff) * (targetGR - state.gainReductionDb);
			}

			// Apply gain to the delayed sample + makeup gain
			const float gainLinear = juce::Decibels::decibelsToGain(-state.gainReductionDb) * makeupLinear;
			out[n] = delayedSample * gainLinear;
		}
	}
}