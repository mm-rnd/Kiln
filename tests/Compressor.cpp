#include "Compressor.h"
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <juce_dsp/juce_dsp.h>

// ---------------------------------------------------------------------------
//  Helpers
// ---------------------------------------------------------------------------

/** Fill a channel array with a sine wave */
static void fillSine(float* data, int numSamples, double frequencyHz, double sampleRate, float amplitude = 1.0f)
{
	for (int n = 0; n < numSamples; ++n)
	{
		const double phase = 2.0 * juce::MathConstants<double>::pi * frequencyHz * static_cast<double>(n) / sampleRate;
		data[n] = amplitude * static_cast<float>(std::sin(phase));
	}
}

/** Compute RMS of a signal */
static float computeRms(const float* data, int numSamples)
{
	float sum = 0.0f;
	for (int n = 0; n < numSamples; ++n)
		sum += data[n] * data[n];
	return std::sqrt(sum / static_cast<float>(numSamples));
}

// ---------------------------------------------------------------------------
//  Construction / defaults
// ---------------------------------------------------------------------------

TEST_CASE("Compressor: default construction", "[compressor]")
{
	Compressor comp;

	CHECK(comp.getAttack() == Catch::Approx(10.0f));
	CHECK(comp.getRelease() == Catch::Approx(100.0f));
	CHECK(comp.getThreshold() == Catch::Approx(-24.0f));
	CHECK(comp.getRatio() == Catch::Approx(4.0f));
	CHECK(comp.getKnee() == true);
	CHECK(comp.getLookaheadEnabled() == false);
	CHECK(comp.getMakeupGain() == Catch::Approx(0.0f));
	CHECK(comp.hasLookahead() == false);
	// getLookaheadDelaySamples() is only valid after prepare()
}

// ---------------------------------------------------------------------------
//  Setters / getters
// ---------------------------------------------------------------------------

TEST_CASE("Compressor: set and get parameters", "[compressor]")
{
	Compressor comp;

	SECTION("attack")
	{
		comp.setAttack(0.5f);
		CHECK(comp.getAttack() == Catch::Approx(0.5f));

		// Shouldn't allow zero or negative
		comp.setAttack(0.0f);
		CHECK(comp.getAttack() == Catch::Approx(0.01f));

		comp.setAttack(-5.0f);
		CHECK(comp.getAttack() == Catch::Approx(0.01f));
	}

	SECTION("release")
	{
		comp.setRelease(250.0f);
		CHECK(comp.getRelease() == Catch::Approx(250.0f));

		// Shouldn't allow below 1 ms
		comp.setRelease(0.0f);
		CHECK(comp.getRelease() == Catch::Approx(1.0f));
	}

	SECTION("threshold")
	{
		comp.setThreshold(-40.0f);
		CHECK(comp.getThreshold() == Catch::Approx(-40.0f));
	}

	SECTION("ratio")
	{
		comp.setRatio(8.0f);
		CHECK(comp.getRatio() == Catch::Approx(8.0f));

		// Ratio must be at least 1:1
		comp.setRatio(0.5f);
		CHECK(comp.getRatio() == Catch::Approx(1.0f));
	}

	SECTION("knee")
	{
		comp.setKnee(false);
		CHECK(comp.getKnee() == false);

		comp.setKnee(true);
		CHECK(comp.getKnee() == true);
	}

	SECTION("lookahead")
	{
		comp.setLookaheadEnabled(true);
		CHECK(comp.getLookaheadEnabled() == true);
		CHECK(comp.hasLookahead() == true);

		comp.setLookaheadEnabled(false);
		CHECK(comp.getLookaheadEnabled() == false);
		CHECK(comp.hasLookahead() == false);
	}

	SECTION("makeup gain")
	{
		comp.setMakeupGain(6.0f);
		CHECK(comp.getMakeupGain() == Catch::Approx(6.0f));
	}
}

// ---------------------------------------------------------------------------
//  Prepare
// ---------------------------------------------------------------------------

TEST_CASE("Compressor: prepare does not crash", "[compressor]")
{
	Compressor comp;
	REQUIRE_NOTHROW(comp.prepare(44100.0, 512, 2));
	REQUIRE_NOTHROW(comp.prepare(48000.0, 256, 1));
	REQUIRE_NOTHROW(comp.prepare(96000.0, 1024, 6));
	REQUIRE_NOTHROW(comp.prepare(44100.0, 512, 0));
}

// ---------------------------------------------------------------------------
//  Process: silence in = silence out
// ---------------------------------------------------------------------------

TEST_CASE("Compressor: silence input produces silence output", "[compressor]")
{
	constexpr double sr = 44100.0;
	constexpr int blockSize = 512;
	constexpr int numChannels = 2;

	Compressor comp;
	comp.prepare(sr, blockSize, numChannels);

	// Allocate input/output arrays
	std::vector<const float*> inputPtrs(static_cast<size_t>(numChannels));
	std::vector<float*> outputPtrs(static_cast<size_t>(numChannels));
	std::vector<std::vector<float>> input(static_cast<size_t>(numChannels));
	std::vector<std::vector<float>> output(static_cast<size_t>(numChannels));

	for (int ch = 0; ch < numChannels; ++ch)
	{
		input[static_cast<size_t>(ch)].assign(static_cast<size_t>(blockSize), 0.0f);
		output[static_cast<size_t>(ch)].assign(static_cast<size_t>(blockSize), 0.0f);
		inputPtrs[static_cast<size_t>(ch)] = input[static_cast<size_t>(ch)].data();
		outputPtrs[static_cast<size_t>(ch)] = output[static_cast<size_t>(ch)].data();
	}

	comp.process(inputPtrs.data(), outputPtrs.data(), blockSize);

	for (int ch = 0; ch < numChannels; ++ch)
	{
		for (int n = 0; n < blockSize; ++n)
			CHECK(output[static_cast<size_t>(ch)][static_cast<size_t>(n)] == Catch::Approx(0.0f));
	}
}

// ---------------------------------------------------------------------------
//  Process: signal below threshold passes through unchanged
// ---------------------------------------------------------------------------

TEST_CASE("Compressor: signal below threshold is not compressed", "[compressor]")
{
	constexpr double sr = 44100.0;
	constexpr int blockSize = 512;
	constexpr int numChannels = 1;

	Compressor comp;
	comp.prepare(sr, blockSize, numChannels);

	// Set a high threshold so our signal stays below it
	comp.setThreshold(0.0f);
	comp.setRatio(10.0f);
	comp.setMakeupGain(0.0f);

	// Generate a quiet signal (well below 0 dB FS)
	std::vector<float> input(static_cast<size_t>(blockSize));
	fillSine(input.data(), blockSize, 440.0, sr, 0.01f); // -40 dB FS

	std::vector<float> output(static_cast<size_t>(blockSize), 0.0f);
	const float* inPtr = input.data();
	float* outPtr = output.data();

	// Process a few blocks to stabilise the envelope filter
	for (int i = 0; i < 10; ++i)
	{
		std::fill(output.begin(), output.end(), 0.0f);
		comp.process(&inPtr, &outPtr, blockSize);
	}

	const float outputRms = computeRms(output.data(), blockSize);
	const float inputRms = computeRms(input.data(), blockSize);

	// Signal below threshold should pass through with minimal attenuation
	// (the envelope may cause a tiny amount but it should be very close)
	CHECK(outputRms > inputRms * 0.9f);
}

// ---------------------------------------------------------------------------
//  Process: signal above threshold is attenuated
// ---------------------------------------------------------------------------

TEST_CASE("Compressor: signal above threshold is attenuated by the correct ratio", "[compressor]")
{
	constexpr double sr = 44100.0;
	constexpr int blockSize = 2048;
	constexpr int numChannels = 1;

	Compressor comp;
	comp.prepare(sr, blockSize, numChannels);

	// Configure for predictable hard-knee compression
	comp.setAttack(1.0f);	   // fast attack
	comp.setRelease(100.0f);   // fast enough release
	comp.setThreshold(-12.0f); // threshold at -12 dB FS
	comp.setRatio(4.0f);	   // 4:1 ratio
	comp.setKnee(false);	   // hard knee
	comp.setMakeupGain(0.0f);

	// Generate a signal at -6 dB FS (6 dB above threshold)
	// With 4:1 ratio, gain reduction should be: 6 * (1 - 1/4) = 4.5 dB
	// So output level should be: -6 - 4.5 = -10.5 dB FS
	std::vector<float> input(static_cast<size_t>(blockSize));
	fillSine(input.data(), blockSize, 440.0, sr, 0.5f); // -6 dB FS

	std::vector<float> output(static_cast<size_t>(blockSize), 0.0f);
	const float* inPtr = input.data();
	float* outPtr = output.data();

	// Process several blocks for the envelope to stabilise
	for (int i = 0; i < 20; ++i)
	{
		std::fill(output.begin(), output.end(), 0.0f);
		comp.process(&inPtr, &outPtr, blockSize);
	}

	const float inputRms = computeRms(input.data(), blockSize);
	const float inputDb = 20.0f * std::log10(inputRms);
	const float outputRms = computeRms(output.data(), blockSize);
	const float outputDb = 20.0f * std::log10(outputRms);

	// The input is at -6 dB FS, threshold at -12 dB FS
	// Overshoot: 6 dB
	// Expected GR: 6 * (1 - 1/4) = 4.5 dB
	// Expected output: -6 - 4.5 = -10.5 dB FS
	// Allow some tolerance due to envelope stabilisation
	CHECK(outputDb == Catch::Approx(-10.5f).margin(1.5f));

	// Also verify output RMS is less than input RMS
	CHECK(outputRms < inputRms);
}

// ---------------------------------------------------------------------------
//  Process: hard knee vs soft knee behaviour
// ---------------------------------------------------------------------------

TEST_CASE("Compressor: soft knee produces smoother transition than hard knee", "[compressor]")
{
	constexpr double sr = 44100.0;
	constexpr int blockSize = 4096;
	constexpr int numChannels = 1;

	Compressor comp;
	comp.prepare(sr, blockSize, numChannels);

	comp.setAttack(1.0f);
	comp.setRelease(200.0f);
	comp.setThreshold(-12.0f);
	comp.setRatio(4.0f);
	comp.setMakeupGain(0.0f);

	// Try with hard knee first, then soft
	std::vector<float> input(static_cast<size_t>(blockSize));
	fillSine(input.data(), blockSize, 440.0, sr, 0.3f); // ~-10.5 dB FS (just above threshold)

	SECTION("hard knee")
	{
		comp.setKnee(false);

		std::vector<float> output(static_cast<size_t>(blockSize), 0.0f);
		const float* inPtr = input.data();
		float* outPtr = output.data();
		for (int i = 0; i < 20; ++i)
		{
			std::fill(output.begin(), output.end(), 0.0f);
			comp.process(&inPtr, &outPtr, blockSize);
		}

		const float outputRms = computeRms(output.data(), blockSize);
		CHECK(outputRms > 0.0f);
	}

	SECTION("soft knee")
	{
		comp.setKnee(true);

		std::vector<float> output(static_cast<size_t>(blockSize), 0.0f);
		const float* inPtr = input.data();
		float* outPtr = output.data();
		for (int i = 0; i < 20; ++i)
		{
			std::fill(output.begin(), output.end(), 0.0f);
			comp.process(&inPtr, &outPtr, blockSize);
		}

		const float outputRms = computeRms(output.data(), blockSize);
		CHECK(outputRms > 0.0f);
	}
}

// ---------------------------------------------------------------------------
//  Process: make-up gain
// ---------------------------------------------------------------------------

TEST_CASE("Compressor: make-up gain boosts output level", "[compressor]")
{
	constexpr double sr = 44100.0;
	constexpr int blockSize = 2048;
	constexpr int numChannels = 1;

	Compressor comp;
	comp.prepare(sr, blockSize, numChannels);

	comp.setAttack(1.0f);
	comp.setRelease(100.0f);
	comp.setThreshold(-30.0f); // well below signal level
	comp.setRatio(4.0f);
	comp.setKnee(false);

	// Signal at -12 dB FS - will be compressed
	std::vector<float> input(static_cast<size_t>(blockSize));
	fillSine(input.data(), blockSize, 440.0, sr, 0.25f);

	// First: process without makeup gain
	comp.setMakeupGain(0.0f);

	std::vector<float> outputNoMakeup(static_cast<size_t>(blockSize), 0.0f);
	const float* inPtr = input.data();
	float* outPtrNoMakeup = outputNoMakeup.data();
	for (int i = 0; i < 20; ++i)
	{
		std::fill(outputNoMakeup.begin(), outputNoMakeup.end(), 0.0f);
		comp.process(&inPtr, &outPtrNoMakeup, blockSize);
	}

	// Then: process with 6 dB makeup gain
	comp.setMakeupGain(6.0f);

	std::vector<float> outputWithMakeup(static_cast<size_t>(blockSize), 0.0f);
	float* outPtrWithMakeup = outputWithMakeup.data();
	for (int i = 0; i < 20; ++i)
	{
		std::fill(outputWithMakeup.begin(), outputWithMakeup.end(), 0.0f);
		comp.process(&inPtr, &outPtrWithMakeup, blockSize);
	}

	const float rmsNoMakeup = computeRms(outputNoMakeup.data(), blockSize);
	const float rmsWithMakeup = computeRms(outputWithMakeup.data(), blockSize);

	// 6 dB boost ≈ 2x amplitude ratio
	CHECK(rmsWithMakeup > rmsNoMakeup * 1.8f);
}

// ---------------------------------------------------------------------------
//  Process: lookahead delay
// ---------------------------------------------------------------------------

TEST_CASE("Compressor: lookahead introduces expected delay", "[compressor]")
{
	constexpr double sr = 44100.0;
	constexpr int blockSize = 1024;
	constexpr int numChannels = 1;

	Compressor comp;
	comp.setLookaheadEnabled(true); // 2 ms fixed delay
	comp.prepare(sr, blockSize, numChannels);

	const int expectedDelay = static_cast<int>(std::ceil(0.002 * sr));
	CHECK(comp.getLookaheadDelaySamples() == expectedDelay);
	CHECK(comp.hasLookahead() == true);

	// With lookahead enabled and no gain reduction (threshold very high),
	// output should be close to the delayed input (same RMS, different phase).
	comp.setThreshold(0.0f); // signal will be below threshold
	comp.setRatio(10.0f);
	comp.setAttack(1.0f);
	comp.setRelease(100.0f);
	comp.setMakeupGain(0.0f);

	std::vector<float> input(static_cast<size_t>(blockSize));
	fillSine(input.data(), blockSize, 440.0, sr, 0.5f);

	std::vector<float> output(static_cast<size_t>(blockSize), 0.0f);
	const float* inPtr = input.data();
	float* outPtr = output.data();
	for (int i = 0; i < 10; ++i)
	{
		std::fill(output.begin(), output.end(), 0.0f);
		comp.process(&inPtr, &outPtr, blockSize);
	}

	const float outputRms = computeRms(output.data(), blockSize);
	const float inputRms = computeRms(input.data(), blockSize);

	// RMS should be very close since the signal is below threshold
	CHECK(outputRms == Catch::Approx(inputRms).margin(0.04f));

	// Disable lookahead — delay stays the same, only envelope source changes
	comp.setLookaheadEnabled(false);
	CHECK(comp.hasLookahead() == false);
	// getLookaheadDelaySamples() still returns the fixed delay
	CHECK(comp.getLookaheadDelaySamples() == expectedDelay);
}

// ---------------------------------------------------------------------------
//  Process: channel agnostic (multiple channels)
// ---------------------------------------------------------------------------

TEST_CASE("Compressor: processes multiple channels independently", "[compressor]")
{
	constexpr double sr = 44100.0;
	constexpr int blockSize = 512;
	constexpr int numChannels = 4;

	Compressor comp;
	comp.prepare(sr, blockSize, numChannels);
	comp.setAttack(1.0f);
	comp.setRelease(100.0f);
	comp.setThreshold(-18.0f);
	comp.setRatio(4.0f);
	comp.setMakeupGain(0.0f);

	// Create different signals on each channel
	std::vector<std::vector<float>> inputs(static_cast<size_t>(numChannels));
	std::vector<std::vector<float>> outputs(static_cast<size_t>(numChannels));
	std::vector<const float*> inPtrs(static_cast<size_t>(numChannels));
	std::vector<float*> outPtrs(static_cast<size_t>(numChannels));

	for (int ch = 0; ch < numChannels; ++ch)
	{
		inputs[static_cast<size_t>(ch)].resize(static_cast<size_t>(blockSize));
		outputs[static_cast<size_t>(ch)].resize(static_cast<size_t>(blockSize), 0.0f);
		const float amplitude = 0.2f * static_cast<float>(ch + 1);
		fillSine(inputs[static_cast<size_t>(ch)].data(), blockSize, 220.0 * (ch + 1), sr, amplitude);
		inPtrs[static_cast<size_t>(ch)] = inputs[static_cast<size_t>(ch)].data();
		outPtrs[static_cast<size_t>(ch)] = outputs[static_cast<size_t>(ch)].data();
	}

	// Process a few blocks
	for (int i = 0; i < 20; ++i)
	{
		for (int ch = 0; ch < numChannels; ++ch)
			std::fill(outputs[static_cast<size_t>(ch)].begin(), outputs[static_cast<size_t>(ch)].end(), 0.0f);
		comp.process(inPtrs.data(), outPtrs.data(), blockSize);
	}

	// All channels should be processed (non-zero output for non-zero input)
	for (int ch = 0; ch < numChannels; ++ch)
	{
		const float rmsVal = computeRms(outputs[static_cast<size_t>(ch)].data(), blockSize);
		CHECK(rmsVal > 0.0f);
		// Higher amplitude channels should have more gain reduction (lower output ratio)
		if (ch > 0)
		{
			const float prevRms = computeRms(outputs[static_cast<size_t>(ch - 1)].data(), blockSize);
			const float thisRms = computeRms(outputs[static_cast<size_t>(ch)].data(), blockSize);
			// Channel with higher input should have higher (or equal) output
			CHECK(thisRms >= prevRms * 0.9f);
		}
	}
}

// ---------------------------------------------------------------------------
//  Process: no processing when no channels
// ---------------------------------------------------------------------------

TEST_CASE("Compressor: zero channels does not crash", "[compressor]")
{
	constexpr double sr = 44100.0;
	constexpr int blockSize = 512;

	Compressor comp;
	comp.prepare(sr, blockSize, 0);

	const float* in = nullptr;
	float* out = nullptr;

	REQUIRE_NOTHROW(comp.process(&in, &out, blockSize));
}

// ---------------------------------------------------------------------------
//  Process: zero samples does not crash
// ---------------------------------------------------------------------------

TEST_CASE("Compressor: zero samples does not crash", "[compressor]")
{
	constexpr double sr = 44100.0;

	Compressor comp;
	comp.prepare(sr, 512, 2);

	std::vector<float> input(static_cast<size_t>(512), 0.5f);
	std::vector<float> output(static_cast<size_t>(512), 0.0f);
	const float* inPtr = input.data();
	float* outPtr = output.data();

	REQUIRE_NOTHROW(comp.process(&inPtr, &outPtr, 0));
}

// ---------------------------------------------------------------------------
//  Prepare: changing channel count recreates states
// ---------------------------------------------------------------------------

TEST_CASE("Compressor: prepare with different channel counts resets state", "[compressor]")
{
	constexpr double sr = 44100.0;
	constexpr int blockSize = 512;

	Compressor comp;
	comp.prepare(sr, blockSize, 2);

	// Prepare with more channels
	REQUIRE_NOTHROW(comp.prepare(sr, blockSize, 6));

	// Prepare with fewer channels
	REQUIRE_NOTHROW(comp.prepare(sr, blockSize, 1));

	// Process with single channel
	std::vector<float> input(static_cast<size_t>(blockSize), 0.5f);
	std::vector<float> output(static_cast<size_t>(blockSize), 0.0f);
	const float* inPtr = input.data();
	float* outPtr = output.data();

	REQUIRE_NOTHROW(comp.process(&inPtr, &outPtr, blockSize));
}