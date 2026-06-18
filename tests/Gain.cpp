#include "Gain.h"

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

// ---------------------------------------------------------------------------
//  prepare()
// ---------------------------------------------------------------------------

TEST_CASE("Gain::prepare sets sample rate and block size", "[gain]")
{
	Gain g;
	g.prepare(48000.0);
	// No direct getters, but prepare must not crash and must initialise state.
	// We verify indirectly by processing a block.
	juce::AudioBuffer<float> buf(2, 256);
	buf.clear();
	g.setGain(0.0f);
	g.process(buf);
	CHECK(buf.getRMSLevel(0, 0, 256) == 0.0f);
}

// ---------------------------------------------------------------------------
//  setGain / getGain
// ---------------------------------------------------------------------------

TEST_CASE("Gain::setGain clamps to -24..+24 dB", "[gain]")
{
	Gain g;
	g.prepare(48000.0);

	g.setGain(-30.0f);
	CHECK(g.getGain() == -24.0f);

	g.setGain(0.0f);
	CHECK(g.getGain() == 0.0f);

	g.setGain(12.0f);
	CHECK(g.getGain() == 12.0f);

	g.setGain(48.0f);
	CHECK(g.getGain() == 24.0f);
}

// ---------------------------------------------------------------------------
//  process() – unity gain
// ---------------------------------------------------------------------------

TEST_CASE("Gain::process with 0 dB leaves signal unchanged", "[gain]")
{
	Gain g;
	g.prepare(44100.0);
	g.setGain(0.0f);

	juce::AudioBuffer<float> buf(2, 256);
	for (int ch = 0; ch < 2; ++ch)
	{
		auto* d = buf.getWritePointer(ch);
		for (int n = 0; n < 256; ++n)
			d[n] = static_cast<float>(n) / 256.0f;
	}

	g.process(buf);

	for (int ch = 0; ch < 2; ++ch)
	{
		const auto* d = buf.getReadPointer(ch);
		for (int n = 0; n < 256; ++n)
		{
			CHECK(approxEqual(d[n], static_cast<float>(n) / 256.0f));
		}
	}
}

// ---------------------------------------------------------------------------
//  process() – positive gain
// ---------------------------------------------------------------------------

TEST_CASE("Gain::process with +6 dB doubles the signal", "[gain]")
{
	Gain g;
	g.setGain(6.0f);
	g.prepare(44100.0);

	juce::AudioBuffer<float> buf(2, 256);
	for (int ch = 0; ch < 2; ++ch)
	{
		auto* d = buf.getWritePointer(ch);
		for (int n = 0; n < 256; ++n)
			d[n] = 0.5f;
	}

	g.process(buf);

	CHECK(g.getGain() == Approx(6.0f));

	const float expected = 0.5f * dbToLinear(6.0f); // ≈ 2.0
	for (int ch = 0; ch < 2; ++ch)
	{
		const auto* d = buf.getReadPointer(ch);
		for (int n = 0; n < 256; ++n)
		{
			CHECK(d[n] == Approx(expected));
		}
	}
}

// ---------------------------------------------------------------------------
//  process() – negative gain
// ---------------------------------------------------------------------------

TEST_CASE("Gain::process with -20 dB attenuates the signal", "[gain]")
{
	Gain g;
	g.prepare(44100.0);
	g.setGain(-20.0f);

	// Process a larger buffer to allow smoothing to reach the target.
	// With 50 ms smoothing at 44.1 kHz, we need ~2205 samples per time constant.
	juce::AudioBuffer<float> buf(1, 4096);
	for (int n = 0; n < 4096; ++n)
		buf.setSample(0, n, 1.0f);

	g.process(buf);

	const float expected = 1.0f * dbToLinear(-20.0f); // ≈ 0.1
	const auto* d = buf.getReadPointer(0);
	// Check the last sample, after smoothing has had time to converge.
	std::cout << "value: " << d[4095] << std::endl;
	CHECK(approxEqual(d[4095], expected, 0.001));
}

// ---------------------------------------------------------------------------
//  process() – multi-channel
// ---------------------------------------------------------------------------

TEST_CASE("Gain::process applies the same gain to all channels", "[gain]")
{
	Gain g;
	g.setGain(3.0f);
	g.prepare(44100.0);

	CHECK(g.getGain() == Approx(3.0f));

	juce::AudioBuffer<float> buf(4, 128);
	for (int ch = 0; ch < 4; ++ch)
		for (int n = 0; n < 128; ++n)
			buf.setSample(ch, n, 0.25f);

	g.process(buf);

	const float expected = 0.25f * dbToLinear(3.0f);
	for (int ch = 0; ch < 4; ++ch)
	{
		const auto* d = buf.getReadPointer(ch);
		for (int n = 0; n < 128; ++n)
		{
			CHECK(d[n] == Approx(expected));
		}
	}
}

// ---------------------------------------------------------------------------
//  process() – empty buffer
// ---------------------------------------------------------------------------

TEST_CASE("Gain::process handles zero-length buffers without crashing", "[gain]")
{
	Gain g;
	g.prepare(44100.0);
	g.setGain(6.0f);

	juce::AudioBuffer<float> buf(2, 0);
	g.process(buf); // must not throw / crash
}

// ---------------------------------------------------------------------------
//  process() – smoothing (smoothed parameter change)
// ---------------------------------------------------------------------------

TEST_CASE("Gain::process smooths parameter changes over multiple blocks", "[gain]")
{
	Gain g;
	g.prepare(44100.0);
	g.setGain(0.0f);

	// Process an initial block to establish the starting smoothed value.
	juce::AudioBuffer<float> buf(1, 512);
	for (int n = 0; n < 512; ++n)
		buf.setSample(0, n, 1.0f);
	g.process(buf);
	CHECK(approxEqual(buf.getSample(0, 0), 1.0f));

	// Change to +12 dB.
	g.setGain(12.0f);

	// Process many blocks; smoothed value should approach target.
	// With 50 ms smoothing at 44.1 kHz, ~11 000 samples (≈ 250 ms) is enough
	// for the value to converge to within 0.1 % of the target.
	for (int i = 0; i < 30; ++i)
	{
		buf.clear();
		for (int n = 0; n < 512; ++n)
			buf.setSample(0, n, 1.0f);
		g.process(buf);
	}

	const float finalSample = buf.getSample(0, 0);
	const float expected = dbToLinear(12.0f);
	CHECK(approxEqual(finalSample, expected, 1.0e-3f));
}
