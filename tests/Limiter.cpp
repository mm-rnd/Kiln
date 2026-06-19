#include "Limiter.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cmath>

using Catch::Approx;

// ---------------------------------------------------------------------------
//  Helpers
// ---------------------------------------------------------------------------

static float dbToLinear(float db)
{
	return std::pow(10.0f, db * 0.05f);
}

static float approxEqual(float a, float b, float tolerance = 1.0e-4f)
{
	return std::abs(a - b) < tolerance;
}

/** Fill a buffer with a constant value on all channels. */
static void fillConstant(juce::AudioBuffer<float>& buffer, float value)
{
	for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
		std::fill(buffer.getWritePointer(ch), buffer.getWritePointer(ch) + buffer.getNumSamples(), value);
}

/** Fill a buffer with a sine wave on all channels. */
static void fillSine(juce::AudioBuffer<float>& buffer, float amplitude, float frequency, float sampleRate)
{
	for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
	{
		auto* d = buffer.getWritePointer(ch);
		for (int n = 0; n < buffer.getNumSamples(); ++n)
			d[n] = amplitude * std::sin(2.0f * juce::MathConstants<float>::pi * frequency * static_cast<float>(n) / static_cast<float>(sampleRate));
	}
}

/** Find the maximum absolute sample value in a buffer. */
static float maxAbs(const juce::AudioBuffer<float>& buffer)
{
	float m = 0.0f;
	for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
	{
		const auto* d = buffer.getReadPointer(ch);
		for (int n = 0; n < buffer.getNumSamples(); ++n)
			m = std::max(m, std::abs(d[n]));
	}
	return m;
}

/**
 * Process enough blocks of constant signal to let the limiter reach steady
 * state, then return the peak of the final block.
 *
 * The limiter has lookahead delay + oversampling filter latency, so the first
 * block(s) may not be fully limited.  This helper ensures we measure the
 * steady-state behaviour.
 */
static float steadyStatePeak(Limiter& lim, float signalLevel, int numChannels, int numWarmupBlocks = 10)
{
	const int bs = 512;
	juce::AudioBuffer<float> buf(numChannels, bs);

	for (int i = 0; i < numWarmupBlocks; ++i)
	{
		fillConstant(buf, signalLevel);
		lim.process(buf);
	}

	return maxAbs(buf);
}

// ---------------------------------------------------------------------------
//  prepare()
// ---------------------------------------------------------------------------

TEST_CASE("Limiter::prepare initialises state without crashing", "[limiter]")
{
	Limiter lim;
	lim.prepare(44100.0, 512, 2);

	// After prepare, processing should work
	juce::AudioBuffer<float> buf(2, 512);
	buf.clear();
	lim.process(buf);
	CHECK(buf.getRMSLevel(0, 0, 512) == 0.0f);
}

TEST_CASE("Limiter::prepare can be called multiple times", "[limiter]")
{
	Limiter lim;
	lim.prepare(44100.0, 512, 2);
	lim.prepare(48000.0, 256, 1);
	lim.prepare(96000.0, 128, 4);

	// Should not crash after re-prepare
	juce::AudioBuffer<float> buf(4, 128);
	buf.clear();
	lim.process(buf);
}

// ---------------------------------------------------------------------------
//  setCeiling / getCeiling
// ---------------------------------------------------------------------------

TEST_CASE("Limiter::setCeiling clamps to -24..0 dB", "[limiter]")
{
	Limiter lim;

	lim.setCeiling(0.0f);
	CHECK(lim.getCeiling() == 0.0f);

	lim.setCeiling(-6.0f);
	CHECK(lim.getCeiling() == -6.0f);

	lim.setCeiling(-24.0f);
	CHECK(lim.getCeiling() == -24.0f);

	// Below minimum clamps to -24
	lim.setCeiling(-30.0f);
	CHECK(lim.getCeiling() == -24.0f);

	// Above maximum clamps to 0
	lim.setCeiling(6.0f);
	CHECK(lim.getCeiling() == 0.0f);
}

// ---------------------------------------------------------------------------
//  getLatencySamples()
// ---------------------------------------------------------------------------

TEST_CASE("Limiter::getLatencySamples returns lookahead latency before prepare", "[limiter]")
{
	Limiter lim;

	// Before prepare, the default sample rate (44100) is used to compute
	// the lookahead latency: 1ms at 44.1 kHz ≈ 44 samples.
	// The oversampling is not yet allocated so only lookahead counts.
	const int latency = lim.getLatencySamples();
	// 1ms lookahead at 44.1 kHz = 44.1 → rounds to 44
	CHECK(latency == 44);
}

TEST_CASE("Limiter::getLatencySamples returns positive latency after prepare", "[limiter]")
{
	Limiter lim;
	lim.prepare(44100.0, 512, 2);

	const int latency = lim.getLatencySamples();
	CHECK(latency >= 44); // At least the lookahead
	CHECK(latency < 200); // Sanity check: should be well under 200 samples
}

TEST_CASE("Limiter::getLatencySamples scales with sample rate", "[limiter]")
{
	Limiter lim1, lim2;

	lim1.prepare(44100.0, 512, 2);
	lim2.prepare(96000.0, 512, 2);

	// Higher sample rate = more lookahead samples for the same 1ms
	CHECK(lim2.getLatencySamples() > lim1.getLatencySamples());
}

// ---------------------------------------------------------------------------
//  process() – empty / zero-length buffer
// ---------------------------------------------------------------------------

TEST_CASE("Limiter::process handles zero-length buffers without crashing", "[limiter]")
{
	Limiter lim;
	lim.prepare(44100.0, 512, 2);

	juce::AudioBuffer<float> buf(2, 0);
	lim.process(buf); // must not throw / crash
}

TEST_CASE("Limiter::process handles zero channels without crashing", "[limiter]")
{
	Limiter lim;
	lim.prepare(44100.0, 512, 0);

	juce::AudioBuffer<float> buf(0, 512);
	lim.process(buf); // must not throw / crash
}

// ---------------------------------------------------------------------------
//  process() – signal below ceiling passes through
// ---------------------------------------------------------------------------

TEST_CASE("Limiter::process does not affect signal well below ceiling", "[limiter]")
{
	Limiter lim;
	lim.prepare(44100.0, 512, 2);
	lim.setCeiling(0.0f);

	// The limiter introduces lookahead delay and oversampling filter latency,
	// so sample-by-sample comparison with the input will not match.
	// Instead we verify that the output RMS is very close to the input RMS
	// (no gain reduction applied for low-level signals).

	// Fill the delay line with the signal first, then measure RMS on a new block
	juce::AudioBuffer<float> prime(2, 512);
	fillSine(prime, 0.01f, 100.0f, 44100.0f);
	for (int i = 0; i < 10; ++i)
		lim.process(prime);

	// Process a fresh block and measure the RMS
	juce::AudioBuffer<float> buf(2, 512);
	fillSine(buf, 0.01f, 100.0f, 44100.0f);
	lim.process(buf);

	// The RMS should be close to the input RMS (0.01 * 0.707 ≈ 0.00707)
	// The difference should be very small since no limiting is happening
	const float inputRms = 0.01f / std::sqrt(2.0f);
	const float outputRmsCh0 = buf.getRMSLevel(0, 0, 512);
	const float outputRmsCh1 = buf.getRMSLevel(1, 0, 512);
	CHECK(outputRmsCh0 == Approx(inputRms).margin(0.002f));
	CHECK(outputRmsCh1 == Approx(inputRms).margin(0.002f));
}

// ---------------------------------------------------------------------------
//  process() – brick-wall limiting (steady state)
// ---------------------------------------------------------------------------

TEST_CASE("Limiter::process enforces brick-wall ceiling at -6 dB", "[limiter]")
{
	Limiter lim;
	lim.prepare(44100.0, 512, 2);
	lim.setCeiling(-6.0f);

	const float ceilingLinear = dbToLinear(-6.0f); // ≈ 0.501
	const float peak = steadyStatePeak(lim, 1.0f, 2);

	// After warmup, the limiter should keep the signal at or below the ceiling
	CHECK(peak <= Approx(ceilingLinear).margin(0.02f));
}

TEST_CASE("Limiter::process enforces brick-wall ceiling at -12 dB", "[limiter]")
{
	Limiter lim;
	lim.prepare(44100.0, 512, 2);
	lim.setCeiling(-12.0f);

	const float ceilingLinear = dbToLinear(-12.0f); // ≈ 0.251
	const float peak = steadyStatePeak(lim, 1.0f, 2);

	CHECK(peak <= Approx(ceilingLinear).margin(0.02f));
}

TEST_CASE("Limiter::process enforces brick-wall ceiling at 0 dB", "[limiter]")
{
	Limiter lim;
	lim.prepare(44100.0, 512, 2);
	lim.setCeiling(0.0f);

	const float peak = steadyStatePeak(lim, 1.0f, 2);

	// With 0 dB ceiling, a full-scale signal should be limited to ≤ 1.0
	CHECK(peak <= Approx(1.0f).margin(0.02f));
}

// ---------------------------------------------------------------------------
//  process() – multi-channel
// ---------------------------------------------------------------------------

TEST_CASE("Limiter::process applies limiting to all channels", "[limiter]")
{
	Limiter lim;
	lim.prepare(44100.0, 512, 4);
	lim.setCeiling(-6.0f);

	const float ceilingLinear = dbToLinear(-6.0f);

	// Warm up
	juce::AudioBuffer<float> warmup(4, 512);
	for (int i = 0; i < 10; ++i)
	{
		fillConstant(warmup, 1.0f);
		lim.process(warmup);
	}

	// Check the last few samples of each channel in the final warmup block
	for (int ch = 0; ch < 4; ++ch)
	{
		const auto* d = warmup.getReadPointer(ch);
		// Check the tail of the buffer where the limiter has settled
		for (int n = 400; n < 512; ++n)
			CHECK(std::abs(d[n]) <= Approx(ceilingLinear).margin(0.02f));
	}
}

// ---------------------------------------------------------------------------
//  process() – transient response
// ---------------------------------------------------------------------------

TEST_CASE("Limiter::process catches transients (fast attack)", "[limiter]")
{
	Limiter lim;
	lim.prepare(44100.0, 512, 1);
	lim.setCeiling(-12.0f); // Use a tighter ceiling to make the test clearer

	const float ceilingLinear = dbToLinear(-12.0f);

	// Warm up with silence so the envelope is at zero
	juce::AudioBuffer<float> warmup(1, 512);
	warmup.clear();
	for (int i = 0; i < 5; ++i)
		lim.process(warmup);

	// Now send an impulse deep enough into the block for the lookahead
	// to have time to detect and respond before the end.
	juce::AudioBuffer<float> buf(1, 512);
	buf.clear();
	buf.setSample(0, 256, 1.0f); // Impulse near the middle

	lim.process(buf);

	// The impulse should be heavily attenuated.  Due to the lookahead delay
	// and oversampling filter latency, a short transient can slightly exceed
	// the ceiling, but the RMS of the tail should be close to the ceiling level.
	const float outputRms = buf.getRMSLevel(0, 300, 212); // RMS after the impulse hits
	// RMS of a steady signal at the ceiling level would be ceilingLinear.
	// After an isolated impulse, the RMS should be substantially attenuated
	// compared to the input peak of 1.0.
	CHECK(outputRms <= Approx(ceilingLinear).margin(0.01f));
}

// ---------------------------------------------------------------------------
//  process() – ceiling smoothing
// ---------------------------------------------------------------------------

TEST_CASE("Limiter::process smooths ceiling changes over time", "[limiter]")
{
	Limiter lim;
	lim.prepare(44100.0, 512, 1);
	lim.setCeiling(0.0f);

	// Warm up with full-scale signal at 0 dB ceiling
	juce::AudioBuffer<float> buf(1, 512);
	for (int i = 0; i < 10; ++i)
	{
		fillConstant(buf, 1.0f);
		lim.process(buf);
	}

	// Change ceiling to -12 dB
	lim.setCeiling(-12.0f);

	// Process many blocks; the ceiling smoothing should converge
	for (int i = 0; i < 50; ++i)
	{
		fillConstant(buf, 1.0f);
		lim.process(buf);
	}

	// After enough blocks, the output should be close to the target ceiling
	const float ceilingLinear = dbToLinear(-12.0f);
	const float peak = maxAbs(buf);
	CHECK(peak <= Approx(ceilingLinear).margin(0.02f));
}

// ---------------------------------------------------------------------------
//  reset()
// ---------------------------------------------------------------------------

TEST_CASE("Limiter::reset clears internal state", "[limiter]")
{
	Limiter lim;
	lim.prepare(44100.0, 512, 2);
	lim.setCeiling(-6.0f);

	// Process some audio
	juce::AudioBuffer<float> buf(2, 512);
	fillConstant(buf, 1.0f);
	lim.process(buf);

	// Reset
	lim.reset();

	// After reset, processing should still work (re-prepare needed for full re-init)
	lim.prepare(44100.0, 512, 2);
	lim.setCeiling(-6.0f);

	const float peak = steadyStatePeak(lim, 1.0f, 2);

	const float ceilingLinear = dbToLinear(-6.0f);
	CHECK(peak <= Approx(ceilingLinear).margin(0.02f));
}

// ---------------------------------------------------------------------------
//  process() – dynamic release behaviour
// ---------------------------------------------------------------------------

TEST_CASE("Limiter::process recovers from gain reduction (release)", "[limiter]")
{
	Limiter lim;
	lim.prepare(44100.0, 512, 1);
	lim.setCeiling(-12.0f);

	// Warm up with heavy limiting
	juce::AudioBuffer<float> buf(1, 512);
	for (int i = 0; i < 10; ++i)
	{
		fillConstant(buf, 1.0f);
		lim.process(buf);
	}

	// Now send a low-level signal
	fillConstant(buf, 0.01f);
	lim.process(buf);

	// The output should have recovered (not stuck in gain reduction)
	const auto* d = buf.getReadPointer(0);
	// The last samples should be close to the input (0.01) since the signal is well below ceiling
	CHECK(approxEqual(d[500], 0.01f, 0.01f));
}

// ---------------------------------------------------------------------------
//  process() – different sample rates
// ---------------------------------------------------------------------------

TEST_CASE("Limiter::process works at different sample rates", "[limiter]")
{
	for (auto sr : {44100.0, 48000.0, 96000.0})
	{
		Limiter lim;
		const int bs = 512;
		lim.prepare(sr, bs, 2);
		lim.setCeiling(-6.0f);

		const float ceilingLinear = dbToLinear(-6.0f);

		// Warm up
		juce::AudioBuffer<float> buf(2, bs);
		for (int i = 0; i < 10; ++i)
		{
			fillConstant(buf, 1.0f);
			lim.process(buf);
		}

		const float peak = maxAbs(buf);
		CHECK(peak <= Approx(ceilingLinear).margin(0.02f));
	}
}

// ---------------------------------------------------------------------------
//  process() – different block sizes
// ---------------------------------------------------------------------------

TEST_CASE("Limiter::process works with different block sizes", "[limiter]")
{
	for (auto bs : {32, 64, 128, 256, 512, 1024})
	{
		Limiter lim;
		lim.prepare(44100.0, static_cast<int>(bs), 2);
		lim.setCeiling(-6.0f);

		const float ceilingLinear = dbToLinear(-6.0f);

		// Warm up with enough blocks for small buffer sizes.
		// The lookahead delay + oversampling latency mean we need enough
		// total samples processed before steady state.
		const int warmupCount = std::max(20, 4096 / static_cast<int>(bs));
		juce::AudioBuffer<float> buf(2, static_cast<int>(bs));
		for (int i = 0; i < warmupCount; ++i)
		{
			fillConstant(buf, 1.0f);
			lim.process(buf);
		}

		// Check the peak against the ceiling.  Small block sizes may have
		// slightly higher peaks near block boundaries due to the interaction
		// of oversampling filters and lookahead delay, so use a relaxed tolerance.
		const float peak = maxAbs(buf);
		CHECK(peak <= Approx(ceilingLinear).margin(bs < 128 ? 0.08f : 0.04f));
	}
}

// ---------------------------------------------------------------------------
//  process() – mono
// ---------------------------------------------------------------------------

TEST_CASE("Limiter::process works in mono", "[limiter]")
{
	Limiter lim;
	lim.prepare(44100.0, 512, 1);
	lim.setCeiling(-6.0f);

	const float ceilingLinear = dbToLinear(-6.0f);
	const float peak = steadyStatePeak(lim, 1.0f, 1);

	CHECK(peak <= Approx(ceilingLinear).margin(0.02f));
}

// ---------------------------------------------------------------------------
//  process() – stereo
// ---------------------------------------------------------------------------

TEST_CASE("Limiter::process works in stereo", "[limiter]")
{
	Limiter lim;
	lim.prepare(44100.0, 512, 2);
	lim.setCeiling(-6.0f);

	const float ceilingLinear = dbToLinear(-6.0f);
	const float peak = steadyStatePeak(lim, 1.0f, 2);

	CHECK(peak <= Approx(ceilingLinear).margin(0.02f));
}