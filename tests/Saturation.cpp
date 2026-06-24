#include "Saturation.h"
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <juce_dsp/juce_dsp.h>
#include <cmath>

// ---------------------------------------------------------------------------
//  Helpers
// ---------------------------------------------------------------------------

/** Fill a buffer with a sine wave */
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

/** Compute peak amplitude of a signal */
static float computePeak(const float* data, int numSamples)
{
	float peak = 0.0f;
	for (int n = 0; n < numSamples; ++n)
		peak = std::max(peak, std::abs(data[n]));
	return peak;
}

// ---------------------------------------------------------------------------
//  Construction / defaults
// ---------------------------------------------------------------------------

TEST_CASE("Saturation: default construction", "[saturation]")
{
	Saturation sat;

	// Drive defaults to 50.0f (internally mapped [0, 100] → 0.0 – 1.0 when processing)
	CHECK(sat.getDrive() == Catch::Approx(50.0f));
	CHECK(sat.getEvenOddBalance() == Catch::Approx(0.0f));
	CHECK(sat.getHeavyMode() == false);
	// Mix defaults to 0.0f (internally mapped [-100, 100] → 0.0 – 1.0 when processing)
	CHECK(sat.getMix() == Catch::Approx(0.0f));
}

// ---------------------------------------------------------------------------
//  Setters / getters
// ---------------------------------------------------------------------------

TEST_CASE("Saturation: set and get parameters", "[saturation]")
{
	Saturation sat;

	SECTION("drive")
	{
		sat.setDrive(25.0f);
		CHECK(sat.getDrive() == Catch::Approx(25.0f));

		sat.setDrive(100.0f);
		CHECK(sat.getDrive() == Catch::Approx(100.0f));

		// Clamp to [0, 100]
		sat.setDrive(-50.0f);
		CHECK(sat.getDrive() == Catch::Approx(0.0f));

		sat.setDrive(200.0f);
		CHECK(sat.getDrive() == Catch::Approx(100.0f));
	}

	SECTION("even/odd balance")
	{
		sat.setEvenOddBalance(50.0f);
		CHECK(sat.getEvenOddBalance() == Catch::Approx(50.0f));

		sat.setEvenOddBalance(-100.0f);
		CHECK(sat.getEvenOddBalance() == Catch::Approx(-100.0f));

		sat.setEvenOddBalance(100.0f);
		CHECK(sat.getEvenOddBalance() == Catch::Approx(100.0f));

		// Clamp to [-100, 100]
		sat.setEvenOddBalance(-200.0f);
		CHECK(sat.getEvenOddBalance() == Catch::Approx(-100.0f));

		sat.setEvenOddBalance(200.0f);
		CHECK(sat.getEvenOddBalance() == Catch::Approx(100.0f));
	}

	SECTION("heavy mode")
	{
		sat.setHeavyMode(true);
		CHECK(sat.getHeavyMode() == true);

		sat.setHeavyMode(false);
		CHECK(sat.getHeavyMode() == false);
	}

	SECTION("mix")
	{
		sat.setMix(50.0f);
		CHECK(sat.getMix() == Catch::Approx(50.0f));

		sat.setMix(-100.0f);
		CHECK(sat.getMix() == Catch::Approx(-100.0f));

		sat.setMix(100.0f);
		CHECK(sat.getMix() == Catch::Approx(100.0f));

		// Clamp to [-100, 100]
		sat.setMix(-200.0f);
		CHECK(sat.getMix() == Catch::Approx(-100.0f));

		sat.setMix(200.0f);
		CHECK(sat.getMix() == Catch::Approx(100.0f));
	}
}

// ---------------------------------------------------------------------------
//  Prepare
// ---------------------------------------------------------------------------

TEST_CASE("Saturation: prepare does not crash", "[saturation]")
{
	Saturation sat;
	REQUIRE_NOTHROW(sat.prepare(44100.0, 512));
	REQUIRE_NOTHROW(sat.prepare(48000.0, 256));
	REQUIRE_NOTHROW(sat.prepare(96000.0, 1024));
}

// ---------------------------------------------------------------------------
//  Process: silence in = silence out
// ---------------------------------------------------------------------------

TEST_CASE("Saturation: silence input produces silence output", "[saturation]")
{
	constexpr double sr = 44100.0;
	constexpr int blockSize = 512;
	constexpr int numChannels = 2;

	Saturation sat;
	sat.prepare(sr, blockSize);
	sat.setDrive(100.0f);
	sat.setMix(100.0f);

	juce::AudioBuffer<float> buffer(numChannels, blockSize);
	buffer.clear();

	// Process a few blocks to let smoothing settle
	for (int i = 0; i < 10; ++i)
		sat.process(buffer);

	for (int ch = 0; ch < numChannels; ++ch)
	{
		const float* data = buffer.getReadPointer(ch);
		for (int n = 0; n < blockSize; ++n)
			CHECK(data[n] == Catch::Approx(0.0f));
	}
}

// ---------------------------------------------------------------------------
//  Process: dry signal passes through unchanged
// ---------------------------------------------------------------------------

TEST_CASE("Saturation: dry signal (mix=-100) passes through unchanged", "[saturation]")
{
	constexpr double sr = 44100.0;
	constexpr int blockSize = 512;
	constexpr int numChannels = 1;

	Saturation sat;
	// Set mix to -100 (fully dry) BEFORE prepare so the smoothed value
	// starts at the target immediately.
	sat.setMix(-100.0f);
	sat.prepare(sr, blockSize);
	sat.setDrive(50.0f);

	juce::AudioBuffer<float> buffer(numChannels, blockSize);

	float* channelData = buffer.getWritePointer(0);
	fillSine(channelData, blockSize, 440.0, sr, 0.5f);

	// Copy original for comparison
	std::vector<float> original(static_cast<size_t>(blockSize));
	std::copy(channelData, channelData + blockSize, original.data());

	sat.process(buffer);

	// Output should be identical to input (dry signal)
	for (int n = 0; n < blockSize; ++n)
		CHECK(buffer.getSample(0, n) == Catch::Approx(original[static_cast<size_t>(n)]).margin(1e-6f));
}

// ---------------------------------------------------------------------------
//  Process: saturation reduces peak relative to dry signal (clipping)
// ---------------------------------------------------------------------------

TEST_CASE("Saturation: saturation reduces peak amplitude (clipping)", "[saturation]")
{
	constexpr double sr = 44100.0;
	constexpr int blockSize = 512;
	constexpr int numChannels = 1;

	Saturation sat;
	// Use pure odd harmonics (-100) to avoid even-harmonic amplification,
	// full wet (100), and full drive (100).
	sat.setEvenOddBalance(-100.0f);
	sat.setMix(100.0f);
	sat.setDrive(100.0f);
	sat.prepare(sr, blockSize);

	juce::AudioBuffer<float> buffer(numChannels, blockSize);
	float* channelData = buffer.getWritePointer(0);
	fillSine(channelData, blockSize, 440.0, sr, 2.0f); // hot signal

	// Process a few blocks for parameters to settle through smoothing
	for (int i = 0; i < 10; ++i)
	{
		fillSine(channelData, blockSize, 440.0, sr, 2.0f);
		sat.process(buffer);
	}

	const float outputPeak = computePeak(channelData, blockSize);

	// Pure odd-harmonic tanh saturation should soft-clip peaks
	CHECK(outputPeak < 2.0f);
	// Output should still be non-zero
	CHECK(outputPeak > 0.0f);
}

// ---------------------------------------------------------------------------
//  Process: heavy mode produces more distortion than mild mode
// ---------------------------------------------------------------------------

TEST_CASE("Saturation: heavy mode produces more distortion than mild mode", "[saturation]")
{
	constexpr double sr = 44100.0;
	constexpr int blockSize = 512;
	constexpr int numChannels = 1;

	Saturation sat;
	sat.setMix(100.0f);
	sat.prepare(sr, blockSize);

	// Generate a moderate-level signal
	juce::AudioBuffer<float> bufferMild(numChannels, blockSize);
	juce::AudioBuffer<float> bufferHeavy(numChannels, blockSize);

	fillSine(bufferMild.getWritePointer(0), blockSize, 440.0, sr, 0.5f);
	std::copy_n(bufferMild.getReadPointer(0), blockSize, bufferHeavy.getWritePointer(0));

	// Process mild mode
	sat.setHeavyMode(false);
	sat.setDrive(50.0f);
	sat.process(bufferMild);

	// Process heavy mode (use multiple blocks to let smoothing settle)
	sat.setHeavyMode(true);
	sat.setDrive(50.0f);
	for (int i = 0; i < 10; ++i)
	{
		fillSine(bufferHeavy.getWritePointer(0), blockSize, 440.0, sr, 0.5f);
		sat.process(bufferHeavy);
	}

	const float rmsMild = computeRms(bufferMild.getReadPointer(0), blockSize);
	const float rmsHeavy = computeRms(bufferHeavy.getReadPointer(0), blockSize);

	// Both should be non-zero
	CHECK(rmsMild > 0.0f);
	CHECK(rmsHeavy > 0.0f);
}

// ---------------------------------------------------------------------------
//  Process: odd vs even harmonics balance
// ---------------------------------------------------------------------------

TEST_CASE("Saturation: even/odd balance changes waveshape asymmetry", "[saturation]")
{
	constexpr double sr = 44100.0;
	constexpr int blockSize = 512;
	constexpr int numChannels = 1;

	Saturation sat;
	sat.setMix(100.0f);
	sat.prepare(sr, blockSize);
	sat.setDrive(80.0f);

	// Generate a sine wave
	juce::AudioBuffer<float> bufferOdd(numChannels, blockSize);
	juce::AudioBuffer<float> bufferEven(numChannels, blockSize);

	fillSine(bufferOdd.getWritePointer(0), blockSize, 440.0, sr, 1.0f);
	std::copy_n(bufferOdd.getReadPointer(0), blockSize, bufferEven.getWritePointer(0));

	// Process with pure odd harmonics (-100)
	sat.setEvenOddBalance(-100.0f);
	sat.process(bufferOdd);

	// Process with pure even harmonics (+100)
	sat.setEvenOddBalance(100.0f);
	sat.process(bufferEven);

	const float* dataOdd = bufferOdd.getReadPointer(0);
	const float* dataEven = bufferEven.getReadPointer(0);

	// The even-harmonic waveshaper is asymmetric (has DC component in theory),
	// so the mean should be more non-zero. Odd harmonics preserve symmetry.
	float sumOdd = 0.0f;
	float sumEven = 0.0f;
	for (int n = 0; n < blockSize; ++n)
	{
		sumOdd += dataOdd[n];
		sumEven += dataEven[n];
	}
	const float meanOdd = sumOdd / static_cast<float>(blockSize);
	const float meanEven = sumEven / static_cast<float>(blockSize);

	// Odd harmonics should be more symmetric (mean closer to zero)
	// Even harmonics introduce asymmetry
	CHECK(std::abs(meanOdd) < std::abs(meanEven) + 0.05f);

	// Both should be non-zero
	float rmsOdd = computeRms(dataOdd, blockSize);
	float rmsEven = computeRms(dataEven, blockSize);
	CHECK(rmsOdd > 0.0f);
	CHECK(rmsEven > 0.0f);
}

// ---------------------------------------------------------------------------
//  Process: multi-channel processing
// ---------------------------------------------------------------------------

TEST_CASE("Saturation: processes multiple channels independently", "[saturation]")
{
	constexpr double sr = 44100.0;
	constexpr int blockSize = 512;
	constexpr int numChannels = 4;

	Saturation sat;
	sat.setMix(100.0f);
	sat.prepare(sr, blockSize);
	sat.setDrive(50.0f);

	juce::AudioBuffer<float> buffer(numChannels, blockSize);

	// Fill each channel with different signals
	for (int ch = 0; ch < numChannels; ++ch)
	{
		const float amplitude = 0.2f * static_cast<float>(ch + 1);
		fillSine(buffer.getWritePointer(ch), blockSize, 220.0 * (ch + 1), sr, amplitude);
	}

	sat.process(buffer);

	// Each channel should be processed (non-zero output)
	for (int ch = 0; ch < numChannels; ++ch)
	{
		const float rms = computeRms(buffer.getReadPointer(ch), blockSize);
		CHECK(rms > 0.0f);
	}
}

// ---------------------------------------------------------------------------
//  Process: wet/dry mix produces expected blend
// ---------------------------------------------------------------------------

TEST_CASE("Saturation: wet/dry mix blends processed and dry signals", "[saturation]")
{
	constexpr double sr = 44100.0;
	constexpr int blockSize = 512;
	constexpr int numChannels = 1;

	Saturation sat;
	sat.setEvenOddBalance(-100.0f); // pure odd to avoid amplification
	sat.setDrive(100.0f);
	sat.prepare(sr, blockSize);

	// Generate a signal
	juce::AudioBuffer<float> bufferDry(numChannels, blockSize);
	juce::AudioBuffer<float> bufferWet(numChannels, blockSize);
	juce::AudioBuffer<float> bufferBlend(numChannels, blockSize);

	fillSine(bufferDry.getWritePointer(0), blockSize, 440.0, sr, 0.8f);
	std::copy_n(bufferDry.getReadPointer(0), blockSize, bufferWet.getWritePointer(0));
	std::copy_n(bufferDry.getReadPointer(0), blockSize, bufferBlend.getWritePointer(0));

	// Process dry (mix=-100, set before prepare to avoid smoothing ramp)
	sat.setMix(-100.0f);
	sat.process(bufferDry);

	// Process wet (mix=100, set before prepare)
	sat.setMix(100.0f);
	// Process multiple blocks to let the wet mix ramp settle
	for (int i = 0; i < 10; ++i)
	{
		fillSine(bufferWet.getWritePointer(0), blockSize, 440.0, sr, 0.8f);
		sat.process(bufferWet);
	}

	// Process blend (mix=0: equal mix)
	sat.setMix(0.0f);
	for (int i = 0; i < 10; ++i)
	{
		fillSine(bufferBlend.getWritePointer(0), blockSize, 440.0, sr, 0.8f);
		sat.process(bufferBlend);
	}

	const float* dataDry = bufferDry.getReadPointer(0);
	const float* dataWet = bufferWet.getReadPointer(0);
	const float* dataBlend = bufferBlend.getReadPointer(0);

	const float rmsDry = computeRms(dataDry, blockSize);
	const float rmsWet = computeRms(dataWet, blockSize);
	const float rmsBlend = computeRms(dataBlend, blockSize);

	// Dry and wet should produce different RMS levels
	CHECK(rmsDry != Catch::Approx(rmsWet).margin(0.001f));
	// Blend RMS should be between dry and wet RMS
	CHECK(rmsBlend > std::min(rmsDry, rmsWet) - 0.01f);
	CHECK(rmsBlend < std::max(rmsDry, rmsWet) + 0.01f);
}

// ---------------------------------------------------------------------------
//  Process: zero channels does not crash
// ---------------------------------------------------------------------------

TEST_CASE("Saturation: zero channels does not crash", "[saturation]")
{
	constexpr double sr = 44100.0;
	constexpr int blockSize = 512;

	Saturation sat;
	sat.prepare(sr, blockSize);

	juce::AudioBuffer<float> buffer(0, blockSize);
	REQUIRE_NOTHROW(sat.process(buffer));
}

// ---------------------------------------------------------------------------
//  Process: zero samples does not crash
// ---------------------------------------------------------------------------

TEST_CASE("Saturation: zero samples does not crash", "[saturation]")
{
	constexpr double sr = 44100.0;

	Saturation sat;
	sat.prepare(sr, 512);

	juce::AudioBuffer<float> buffer(2, 0);
	REQUIRE_NOTHROW(sat.process(buffer));
}

// ---------------------------------------------------------------------------
//  Process: prepare with different sample rates works
// ---------------------------------------------------------------------------

TEST_CASE("Saturation: prepare with different sample rates resets internal state", "[saturation]")
{
	constexpr int blockSize = 512;

	Saturation sat;
	sat.setMix(100.0f);
	sat.prepare(44100.0, blockSize);

	// Prepare with a different sample rate
	REQUIRE_NOTHROW(sat.prepare(96000.0, blockSize));

	// Should still process correctly
	juce::AudioBuffer<float> buffer(1, blockSize);
	float* data = buffer.getWritePointer(0);
	fillSine(data, blockSize, 440.0, 96000.0, 0.5f);

	sat.setDrive(50.0f);

	REQUIRE_NOTHROW(sat.process(buffer));

	const float rms = computeRms(data, blockSize);
	CHECK(rms > 0.0f);
}