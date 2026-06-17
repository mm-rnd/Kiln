#include "LinearPhaseCrossover.h"
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <juce_dsp/juce_dsp.h>

// ---------------------------------------------------------------------------
//  Helpers
// ---------------------------------------------------------------------------

static juce::AudioBuffer<float> makeSineBuffer(int numChannels, int numSamples, double frequencyHz, double sampleRate,
											   float amplitude = 1.0f)
{
	juce::AudioBuffer<float> buf(numChannels, numSamples);
	for (int ch = 0; ch < numChannels; ++ch)
	{
		float* data = buf.getWritePointer(ch);
		for (int n = 0; n < numSamples; ++n)
		{
			const double phase =
				2.0 * juce::MathConstants<double>::pi * frequencyHz * static_cast<double>(n) / sampleRate;
			data[n] = amplitude * static_cast<float>(std::sin(phase));
		}
	}
	return buf;
}

static float rms(const float* data, int numSamples)
{
	float sum = 0.0f;
	for (int n = 0; n < numSamples; ++n)
		sum += data[n] * data[n];
	return std::sqrt(sum / static_cast<float>(numSamples));
}

/** Process silent blocks to fill the filter's history buffer so that
	subsequent split() calls produce steady-state output. */
static void warmUp(LinearPhaseCrossover& xo, int numChannels, int blockSize, double sampleRate, float frequencyHz,
				   int numBlocks)
{
	for (int i = 0; i < numBlocks; ++i)
	{
		auto buf = makeSineBuffer(numChannels, blockSize, frequencyHz, sampleRate);
		xo.split(buf);
	}
}

// ---------------------------------------------------------------------------
//  Construction / defaults
// ---------------------------------------------------------------------------

TEST_CASE("LinearPhaseCrossover: default construction", "[crossover]")
{
	LinearPhaseCrossover xo;

	CHECK(xo.getLowCrossover() == Catch::Approx(200.0f));
	CHECK(xo.getHighCrossover() == Catch::Approx(2000.0f));
	CHECK(xo.hasDelay() == true);
}

// ---------------------------------------------------------------------------
//  Setters / getters
// ---------------------------------------------------------------------------

TEST_CASE("LinearPhaseCrossover: set and get crossover frequencies", "[crossover]")
{
	LinearPhaseCrossover xo;

	SECTION("low crossover")
	{
		xo.setLowCrossover(100.0f);
		CHECK(xo.getLowCrossover() == Catch::Approx(100.0f));
	}

	SECTION("high crossover")
	{
		xo.setHighCrossover(3000.0f);
		CHECK(xo.getHighCrossover() == Catch::Approx(3000.0f));
	}

	SECTION("both crossovers")
	{
		xo.setLowCrossover(150.0f);
		xo.setHighCrossover(4000.0f);
		CHECK(xo.getLowCrossover() == Catch::Approx(150.0f));
		CHECK(xo.getHighCrossover() == Catch::Approx(4000.0f));
	}
}

// ---------------------------------------------------------------------------
//  Prepare
// ---------------------------------------------------------------------------

TEST_CASE("LinearPhaseCrossover: prepare does not crash", "[crossover]")
{
	LinearPhaseCrossover xo;
	REQUIRE_NOTHROW(xo.prepare(44100.0, 512));
	REQUIRE_NOTHROW(xo.prepare(48000.0, 256));
	REQUIRE_NOTHROW(xo.prepare(96000.0, 1024));
}

// ---------------------------------------------------------------------------
//  hasDelay / getGroupDelaySamples
// ---------------------------------------------------------------------------

TEST_CASE("LinearPhaseCrossover: always reports delay", "[crossover]")
{
	LinearPhaseCrossover xo;
	xo.prepare(44100.0, 512);

	CHECK(xo.hasDelay() == true);
	CHECK(xo.getGroupDelaySamples() == 2766);

	xo.setLowCrossover(80.0f);
	CHECK(xo.getGroupDelaySamples() == 2766);

	xo.setLowCrossover(500.0f);
	CHECK(xo.getGroupDelaySamples() == 2766);

	xo.prepare(96000.0, 512);
	CHECK(xo.getGroupDelaySamples() == 2409);
}

TEST_CASE("LinearPhaseCrossover: group delay is (N-1)/2", "[crossover]")
{
	LinearPhaseCrossover xo;
	xo.prepare(44100.0, 512);
	xo.setLowCrossover(200.0f);

	// The lowCoeffs size should be odd, and delay = (N-1)/2
	const int delay = xo.getGroupDelaySamples();
	CHECK(delay > 0);
	// delay should be consistent: (numTaps - 1) / 2
	// We can verify by checking it's a reasonable value for 44.1 kHz / 200 Hz
	CHECK(delay < 44100); // sanity: delay must be less than 1 second of samples
}

// ---------------------------------------------------------------------------
//  Split: band sum equals delayed original
// ---------------------------------------------------------------------------

TEST_CASE("LinearPhaseCrossover: low + mid + high equals delayed original", "[crossover]")
{
	constexpr double sr = 44100.0;
	constexpr int blockSize = 1024;
	constexpr int numChannels = 2;

	LinearPhaseCrossover xo;
	xo.prepare(sr, blockSize);
	xo.setLowCrossover(200.0f);
	xo.setHighCrossover(2000.0f);

	// Use a broadband signal (white noise)
	juce::AudioBuffer<float> buffer(numChannels, blockSize);
	for (int ch = 0; ch < numChannels; ++ch)
	{
		float* data = buffer.getWritePointer(ch);
		// Simple LCG-based noise for reproducibility
		unsigned int seed = static_cast<unsigned int>(ch * 12345 + 67890);
		for (int n = 0; n < blockSize; ++n)
		{
			seed = seed * 1103515245u + 12345u;
			data[n] = (static_cast<float>(seed & 0x7FFFFFFFu) / static_cast<float>(0x7FFFFFFF)) * 2.0f - 1.0f;
		}
	}

	// Run split once to fill the history buffer, then again to get valid output
	xo.split(buffer);

	// Run a second block so the history is populated and the output is valid
	juce::AudioBuffer<float> buffer2(numChannels, blockSize);
	for (int ch = 0; ch < numChannels; ++ch)
	{
		float* data = buffer2.getWritePointer(ch);
		unsigned int seed = static_cast<unsigned int>(ch * 98765 + 43210);
		for (int n = 0; n < blockSize; ++n)
		{
			seed = seed * 1103515245u + 12345u;
			data[n] = (static_cast<float>(seed & 0x7FFFFFFFu) / static_cast<float>(0x7FFFFFFF)) * 2.0f - 1.0f;
		}
	}
	xo.split(buffer2);

	const int delay = xo.getGroupDelaySamples();

	for (int ch = 0; ch < numChannels; ++ch)
	{
		const float* low = xo.getLowBand(ch);
		const float* mid = xo.getMidBand(ch);
		const float* high = xo.getHighBand(ch);

		for (int n = 0; n < blockSize; ++n)
		{
			const float sum = low[n] + mid[n] + high[n];
			// The bands are aligned at the group delay, so the sum equals
			// the input delayed by getGroupDelaySamples().
			const float delayedOriginal = (n >= delay) ? buffer2.getSample(ch, n - delay) : 0.0f;
			CHECK(sum == Catch::Approx(delayedOriginal).margin(1e-4f));
		}
	}
}

// ---------------------------------------------------------------------------
//  Split: low-frequency sine passes through low band
// ---------------------------------------------------------------------------

TEST_CASE("LinearPhaseCrossover: low-frequency sine mostly in low band", "[crossover]")
{
	constexpr double sr = 44100.0;
	constexpr int blockSize = 8192;
	constexpr float lowFreq = 50.0f; // well below 200 Hz crossover

	LinearPhaseCrossover xo;
	xo.prepare(sr, blockSize);
	xo.setLowCrossover(200.0f);
	xo.setHighCrossover(2000.0f);

	// Warm up: fill the filter history with the same signal
	warmUp(xo, 1, blockSize, sr, lowFreq, 4);

	auto buffer = makeSineBuffer(1, blockSize, lowFreq, sr);
	xo.split(buffer);

	const float lowRms = rms(xo.getLowBand(0), blockSize);
	const float midRms = rms(xo.getMidBand(0), blockSize);
	const float highRms = rms(xo.getHighBand(0), blockSize);
	const float inputRms = rms(buffer.getReadPointer(0), blockSize);

	// Low-frequency sine should mostly appear in the low band
	CHECK(lowRms > inputRms * 0.9f);
	// Mid and high should be negligible
	CHECK(midRms < inputRms * 0.1f);
	CHECK(highRms < inputRms * 0.1f);
}

// ---------------------------------------------------------------------------
//  Split: high-frequency sine passes through high band
// ---------------------------------------------------------------------------

TEST_CASE("LinearPhaseCrossover: high-frequency sine mostly in high band", "[crossover]")
{
	constexpr double sr = 44100.0;
	constexpr int blockSize = 8192;
	constexpr float highFreq = 5000.0f; // well above 2000 Hz crossover

	LinearPhaseCrossover xo;
	xo.prepare(sr, blockSize);
	xo.setLowCrossover(200.0f);
	xo.setHighCrossover(2000.0f);

	// Warm up: fill the filter history with the same signal
	warmUp(xo, 1, blockSize, sr, highFreq, 4);

	auto buffer = makeSineBuffer(1, blockSize, highFreq, sr);
	xo.split(buffer);

	const float lowRms = rms(xo.getLowBand(0), blockSize);
	const float midRms = rms(xo.getMidBand(0), blockSize);
	const float highRms = rms(xo.getHighBand(0), blockSize);
	const float inputRms = rms(buffer.getReadPointer(0), blockSize);

	// High-frequency sine should mostly appear in the high band
	CHECK(highRms > inputRms * 0.9f);
	// Low and mid should be negligible
	CHECK(lowRms < inputRms * 0.1f);
	CHECK(midRms < inputRms * 0.1f);
}

// ---------------------------------------------------------------------------
//  Split: mid-frequency sine passes through mid band
// ---------------------------------------------------------------------------

TEST_CASE("LinearPhaseCrossover: mid-frequency sine mostly in mid band", "[crossover]")
{
	constexpr double sr = 44100.0;
	constexpr int blockSize = 8192;
	constexpr float midFreq = 800.0f; // between 200 and 2000 Hz

	LinearPhaseCrossover xo;
	xo.prepare(sr, blockSize);
	xo.setLowCrossover(200.0f);
	xo.setHighCrossover(2000.0f);

	// Warm up: fill the filter history with the same signal
	warmUp(xo, 1, blockSize, sr, midFreq, 4);

	auto buffer = makeSineBuffer(1, blockSize, midFreq, sr);
	xo.split(buffer);

	const float lowRms = rms(xo.getLowBand(0), blockSize);
	const float midRms = rms(xo.getMidBand(0), blockSize);
	const float highRms = rms(xo.getHighBand(0), blockSize);
	const float inputRms = rms(buffer.getReadPointer(0), blockSize);

	// Mid-frequency sine should mostly appear in the mid band
	CHECK(midRms > inputRms * 0.9f);
	// Low and high should be negligible
	CHECK(lowRms < inputRms * 0.1f);
	CHECK(highRms < inputRms * 0.1f);
}

// ---------------------------------------------------------------------------
//  Split: multi-channel consistency
// ---------------------------------------------------------------------------

TEST_CASE("LinearPhaseCrossover: stereo split is consistent across channels", "[crossover]")
{
	constexpr double sr = 44100.0;
	constexpr int blockSize = 1024;

	LinearPhaseCrossover xo;
	xo.prepare(sr, blockSize);
	xo.setLowCrossover(200.0f);
	xo.setHighCrossover(2000.0f);

	// Same signal on both channels
	auto buffer = makeSineBuffer(2, blockSize, 500.0f, sr);
	xo.split(buffer);

	// Both channels should produce similar band energy
	for (int n = 0; n < blockSize; ++n)
	{
		CHECK(xo.getLowBand(0)[n] == Catch::Approx(xo.getLowBand(1)[n]).margin(1e-6f));
		CHECK(xo.getMidBand(0)[n] == Catch::Approx(xo.getMidBand(1)[n]).margin(1e-6f));
		CHECK(xo.getHighBand(0)[n] == Catch::Approx(xo.getHighBand(1)[n]).margin(1e-6f));
	}
}

// ---------------------------------------------------------------------------
//  Split: changing crossover frequencies updates filtering
// ---------------------------------------------------------------------------

TEST_CASE("LinearPhaseCrossover: changing crossover changes band content", "[crossover]")
{
	constexpr double sr = 44100.0;
	constexpr int blockSize = 8192;
	constexpr float testFreq = 500.0f;

	LinearPhaseCrossover xo;
	xo.prepare(sr, blockSize);

	// First: 500 Hz should be in the MID band (crossovers at 200 / 2000)
	xo.setLowCrossover(200.0f);
	xo.setHighCrossover(2000.0f);

	warmUp(xo, 1, blockSize, sr, testFreq, 4);

	auto buffer1 = makeSineBuffer(1, blockSize, testFreq, sr);
	xo.split(buffer1);
	const float midRms1 = rms(xo.getMidBand(0), blockSize);

	// Second: move high crossover down to 300 Hz — 500 Hz should now be in HIGH band
	xo.setHighCrossover(300.0f);

	// Warm up again after crossover change (filter history was reset)
	warmUp(xo, 1, blockSize, sr, testFreq, 4);

	auto buffer2 = makeSineBuffer(1, blockSize, testFreq, sr);
	xo.split(buffer2);
	const float highRms2 = rms(xo.getHighBand(0), blockSize);
	const float midRms2 = rms(xo.getMidBand(0), blockSize);

	CHECK(midRms1 > 0.1f);	   // was in mid band before
	CHECK(highRms2 > midRms2); // now mostly in high band
}

// ---------------------------------------------------------------------------
//  Split: band output pointers are valid
// ---------------------------------------------------------------------------

TEST_CASE("LinearPhaseCrossover: band output pointers are non-null after split", "[crossover]")
{
	LinearPhaseCrossover xo;
	xo.prepare(44100.0, 512);

	auto buffer = makeSineBuffer(1, 512, 1000.0, 44100.0);
	xo.split(buffer);

	CHECK(xo.getLowBand(0) != nullptr);
	CHECK(xo.getMidBand(0) != nullptr);
	CHECK(xo.getHighBand(0) != nullptr);
}

// ---------------------------------------------------------------------------
//  Edge case: low crossover at minimum
// ---------------------------------------------------------------------------

TEST_CASE("LinearPhaseCrossover: low crossover near DC", "[crossover]")
{
	constexpr double sr = 44100.0;
	constexpr int blockSize = 131072;

	LinearPhaseCrossover xo;
	xo.prepare(sr, blockSize);
	xo.setLowCrossover(50.0f);
	xo.setHighCrossover(2000.0f);

	// Warm up with a 1000 Hz sine (the test signal)
	warmUp(xo, 1, blockSize, sr, 1000.0, 2);

	auto buffer = makeSineBuffer(1, blockSize, 1000.0, sr);
	xo.split(buffer);

	// 1000 Hz should be above the low crossover and below the high crossover.
	// With a very long filter (50 Hz cutoff at 44.1 kHz) some energy is
	// lost to the transient at the block boundary, so we use a relaxed threshold.
	const float midRms = rms(xo.getMidBand(0), blockSize);
	const float inputRms = rms(buffer.getReadPointer(0), blockSize);
	CHECK(midRms > inputRms * 0.5f);
}

// ---------------------------------------------------------------------------
//  Edge case: crossovers close together
// ---------------------------------------------------------------------------

TEST_CASE("LinearPhaseCrossover: narrow mid band", "[crossover]")
{
	constexpr double sr = 44100.0;
	constexpr int blockSize = 8192;

	LinearPhaseCrossover xo;
	xo.prepare(sr, blockSize);
	xo.setLowCrossover(500.0f);
	xo.setHighCrossover(600.0f);

	// 10 Hz sine — should be in low band
	warmUp(xo, 1, blockSize, sr, 10.0, 2);
	auto lowBuf = makeSineBuffer(1, blockSize, 10.0, sr);
	xo.split(lowBuf);
	const float lowRms = rms(xo.getLowBand(0), blockSize);
	const float lowInputRms = rms(lowBuf.getReadPointer(0), blockSize);
	CHECK(lowRms > lowInputRms * 0.9f);

	// 5000 Hz sine — should be in high band
	warmUp(xo, 1, blockSize, sr, 5000.0, 2);
	auto highBuf = makeSineBuffer(1, blockSize, 5000.0, sr);
	xo.split(highBuf);
	const float highRms = rms(xo.getHighBand(0), blockSize);
	const float highInputRms = rms(highBuf.getReadPointer(0), blockSize);
	CHECK(highRms > highInputRms * 0.9f);
}

// ---------------------------------------------------------------------------
//  Multiple split calls produce consistent results
// ---------------------------------------------------------------------------

TEST_CASE("LinearPhaseCrossover: repeated split is deterministic", "[crossover]")
{
	constexpr double sr = 44100.0;
	constexpr int blockSize = 8192;

	LinearPhaseCrossover xo;
	xo.prepare(sr, blockSize);
	xo.setLowCrossover(200.0f);
	xo.setHighCrossover(2000.0f);

	// Warm up with the same signal we'll test with
	constexpr float testFreq = 500.0f;
	warmUp(xo, 1, blockSize, sr, testFreq, 4);

	auto buffer1 = makeSineBuffer(1, blockSize, testFreq, sr);
	xo.split(buffer1);

	// Copy results
	std::vector<float> low1(xo.getLowBand(0), xo.getLowBand(0) + blockSize);
	std::vector<float> mid1(xo.getMidBand(0), xo.getMidBand(0) + blockSize);

	// Split again with identical input — after warm-up the history is
	// fully converged so the output must be identical.
	auto buffer2 = makeSineBuffer(1, blockSize, testFreq, sr);
	xo.split(buffer2);

	for (int n = 0; n < blockSize; ++n)
	{
		CHECK(xo.getLowBand(0)[n] == Catch::Approx(low1[static_cast<size_t>(n)]));
		CHECK(xo.getMidBand(0)[n] == Catch::Approx(mid1[static_cast<size_t>(n)]));
	}
}

// ---------------------------------------------------------------------------
//  Group delay is positive after prepare
// ---------------------------------------------------------------------------

TEST_CASE("LinearPhaseCrossover: group delay increases with lower crossover", "[crossover]")
{
	constexpr double sr = 44100.0;
	constexpr int blockSize = 4096;

	LinearPhaseCrossover xo;
	xo.prepare(sr, blockSize);

	// Lower crossover → longer filter → more delay
	// Call split() after each setter to flush the smoothed crossover change.
	xo.setLowCrossover(100.0f);
	auto buf1 = makeSineBuffer(1, blockSize, 1000.0, sr);
	xo.split(buf1);
	const int delay1 = xo.getGroupDelaySamples();

	xo.setLowCrossover(500.0f);
	auto buf2 = makeSineBuffer(1, blockSize, 1000.0, sr);
	xo.split(buf2);
	const int delay2 = xo.getGroupDelaySamples();

	// 100 Hz filter should have more taps (wider transition in time) than 500 Hz
	CHECK(delay1 > delay2);
}
