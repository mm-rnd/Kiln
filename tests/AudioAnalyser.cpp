#include "AudioAnalyser.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cmath>

using Catch::Approx;

// ---------------------------------------------------------------------------
//  AnalyserData::pushSample
// ---------------------------------------------------------------------------

TEST_CASE("AnalyserData::pushSample writes samples into ring buffer", "[audioanalyser]")
{
	AnalyserData data;
	std::array<float, 8> buffer{};
	buffer.fill(0.0f);
	std::atomic<int> pos{0};

	// Push a few samples
	data.pushSample(buffer, pos, 1.0f);
	data.pushSample(buffer, pos, 2.0f);
	data.pushSample(buffer, pos, 3.0f);

	// Position should have advanced
	CHECK(pos.load() == 3);

	// Samples should be in the buffer
	CHECK(buffer[0] == 1.0f);
	CHECK(buffer[1] == 2.0f);
	CHECK(buffer[2] == 3.0f);
}

TEST_CASE("AnalyserData::pushSample wraps around at buffer size", "[audioanalyser]")
{
	AnalyserData data;
	std::array<float, 4> buffer{};
	buffer.fill(0.0f);
	std::atomic<int> pos{0};

	// Fill the buffer completely
	for (int i = 0; i < 4; ++i)
		data.pushSample(buffer, pos, static_cast<float>(i + 1));

	CHECK(pos.load() == 0); // Wrapped around

	// Values should be in order
	CHECK(buffer[0] == 1.0f);
	CHECK(buffer[1] == 2.0f);
	CHECK(buffer[2] == 3.0f);
	CHECK(buffer[3] == 4.0f);

	// Push one more to verify wrap
	data.pushSample(buffer, pos, 5.0f);
	CHECK(pos.load() == 1);
	CHECK(buffer[0] == 5.0f); // Overwritten
}

// ---------------------------------------------------------------------------
//  AnalyserData::copyBuffer
// ---------------------------------------------------------------------------

TEST_CASE("AnalyserData::copyBuffer copies ring buffer contiguously", "[audioanalyser]")
{
	AnalyserData data;
	std::array<float, 8> src{};
	src.fill(0.0f);
	std::atomic<int> writePos{0};
	std::array<float, 16> dst{};
	dst.fill(-1.0f);

	// Write some samples
	for (int i = 0; i < 5; ++i)
		data.pushSample(src, writePos, static_cast<float>(i + 1));

	data.copyBuffer<8, 16>(src, writePos, dst);

	// copyBuffer starts from writePos, so with writePos=5 the order is:
	// dst[0]=src[5]=0, dst[1]=src[6]=0, dst[2]=src[7]=0,
	// dst[3]=src[0]=1, dst[4]=src[1]=2, dst[5]=src[2]=3,
	// dst[6]=src[3]=4, dst[7]=src[4]=5
	CHECK(dst[0] == 0.0f);
	CHECK(dst[1] == 0.0f);
	CHECK(dst[2] == 0.0f);
	CHECK(dst[3] == 1.0f);
	CHECK(dst[4] == 2.0f);
	CHECK(dst[5] == 3.0f);
	CHECK(dst[6] == 4.0f);
	CHECK(dst[7] == 5.0f);
	// Padding beyond ring size should be zero
	CHECK(dst[8] == 0.0f);
	CHECK(dst[15] == 0.0f);
}

TEST_CASE("AnalyserData::copyBuffer handles wrapped ring buffer", "[audioanalyser]")
{
	AnalyserData data;
	std::array<float, 4> src{};
	src.fill(0.0f);
	std::atomic<int> writePos{0};
	std::array<float, 8> dst{};
	dst.fill(-1.0f);

	// Fill and wrap
	for (int i = 0; i < 6; ++i)
		data.pushSample(src, writePos, static_cast<float>(i + 1));

	// After 6 pushes into size-4 buffer, writePos=2.
	// copyBuffer starts from writePos, so:
	// dst[0]=src[2]=3, dst[1]=src[3]=4, dst[2]=src[0]=5, dst[3]=src[1]=6
	data.copyBuffer<4, 8>(src, writePos, dst);

	CHECK(dst[0] == 3.0f);
	CHECK(dst[1] == 4.0f);
	CHECK(dst[2] == 5.0f);
	CHECK(dst[3] == 6.0f);
	CHECK(dst[4] == 0.0f); // Padding
	CHECK(dst[7] == 0.0f);
}

// ---------------------------------------------------------------------------
//  AnalyserData::computeRMS
// ---------------------------------------------------------------------------

TEST_CASE("AnalyserData::computeRMS returns -inf for silent buffer", "[audioanalyser]")
{
	AnalyserData data;
	std::array<float, 16> buffer{};
	buffer.fill(0.0f);
	std::atomic<int> writePos{0};

	const float rms = data.computeRMS(buffer, writePos);
	CHECK(rms == Approx(-100.0f).margin(0.1f)); // Clamped to minDb
}

TEST_CASE("AnalyserData::computeRMS computes correct level for constant signal", "[audioanalyser]")
{
	AnalyserData data;
	std::array<float, 16> buffer{};
	std::atomic<int> writePos{0};

	const float amplitude = 0.5f;
	for (int i = 0; i < 16; ++i)
		data.pushSample(buffer, writePos, amplitude);

	const float rms = data.computeRMS(buffer, writePos);
	const float expectedDb = juce::Decibels::gainToDecibels(amplitude);
	CHECK(rms == Approx(expectedDb).margin(0.1f));
}

// ---------------------------------------------------------------------------
//  AudioAnalyser construction / lifetime
// ---------------------------------------------------------------------------

TEST_CASE("AudioAnalyser can be constructed and destroyed", "[audioanalyser]")
{
	AnalyserData data;
	AudioAnalyser::BandColours colours{juce::Colours::red, juce::Colours::green, juce::Colours::blue};

	// Should not throw
	AudioAnalyser analyser(data, colours);
}

TEST_CASE("AudioAnalyser startAnalyser/stopAnalyser does not crash", "[audioanalyser]")
{
	AnalyserData data;
	AudioAnalyser::BandColours colours{juce::Colours::red, juce::Colours::green, juce::Colours::blue};
	AudioAnalyser analyser(data, colours);

	analyser.startAnalyser();
	analyser.stopAnalyser();
	analyser.startAnalyser();
	analyser.stopAnalyser();
}

// ---------------------------------------------------------------------------
//  AudioAnalyser constants
// ---------------------------------------------------------------------------

TEST_CASE("AudioAnalyser FFT sizes are powers of two", "[audioanalyser]")
{
	CHECK((AnalyserData::lowFftSize & (AnalyserData::lowFftSize - 1)) == 0);
	CHECK((AnalyserData::midFftSize & (AnalyserData::midFftSize - 1)) == 0);
	CHECK((AnalyserData::highFftSize & (AnalyserData::highFftSize - 1)) == 0);
}

TEST_CASE("AudioAnalyser FFT buffer sizes are 2x FFT size", "[audioanalyser]")
{
	CHECK(AnalyserData::lowFftBufferSize == 2 * AnalyserData::lowFftSize);
	CHECK(AnalyserData::midFftBufferSize == 2 * AnalyserData::midFftSize);
	CHECK(AnalyserData::highFftBufferSize == 2 * AnalyserData::highFftSize);
}

TEST_CASE("AudioAnalyser numBins is half of FFT size", "[audioanalyser]")
{
	CHECK(AnalyserData::lowNumBins == AnalyserData::lowFftSize / 2);
	CHECK(AnalyserData::midNumBins == AnalyserData::midFftSize / 2);
	CHECK(AnalyserData::highNumBins == AnalyserData::highFftSize / 2);
}

TEST_CASE("AudioAnalyser default atomic values are initialised", "[audioanalyser]")
{
	AnalyserData data;

	CHECK(data.lowLevel.load() == -100.0f);
	CHECK(data.midLevel.load() == -100.0f);
	CHECK(data.highLevel.load() == -100.0f);
	CHECK(data.limiterGR.load() == 0.0f);
	CHECK(data.sampleRate.load() == Approx(44100.0));
}

// ---------------------------------------------------------------------------
//  AudioAnalyser::resized
// ---------------------------------------------------------------------------

TEST_CASE("AudioAnalyser::resized does not crash", "[audioanalyser]")
{
	AnalyserData data;
	AudioAnalyser::BandColours colours{juce::Colours::red, juce::Colours::green, juce::Colours::blue};
	AudioAnalyser analyser(data, colours);

	analyser.setSize(800, 200);
	analyser.resized(); // Should be a no-op but must not crash
}