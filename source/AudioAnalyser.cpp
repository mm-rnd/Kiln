#include "AudioAnalyser.h"
#include <algorithm>
#include <cmath>
#include <juce_dsp/juce_dsp.h>

//==============================================================================
AudioAnalyser::AudioAnalyser(AnalyserData& d, BandColours colours)
	: data(d), bandColours(colours), lowFft(std::make_unique<juce::dsp::FFT>(AnalyserData::lowFftOrder)),
	  midFft(std::make_unique<juce::dsp::FFT>(AnalyserData::midFftOrder)),
	  highFft(std::make_unique<juce::dsp::FFT>(AnalyserData::highFftOrder))
{
}

AudioAnalyser::~AudioAnalyser()
{
	stopTimer();
}

void AudioAnalyser::startAnalyser()
{
	startTimerHz(30);
}

void AudioAnalyser::stopAnalyser()
{
	stopTimer();
}

//==============================================================================
//  Timer callback
//==============================================================================
void AudioAnalyser::timerCallback()
{
	currentSampleRate = data.sampleRate.load(std::memory_order_acquire);

	AnalyserData::copyBuffer<AnalyserData::lowFftSize, AnalyserData::lowFftBufferSize>(
		data.lowPreCompBuffer, data.lowPreCompPos, lowPreCompBuf);
	AnalyserData::copyBuffer<AnalyserData::midFftSize, AnalyserData::midFftBufferSize>(
		data.midPreCompBuffer, data.midPreCompPos, midPreCompBuf);
	AnalyserData::copyBuffer<AnalyserData::highFftSize, AnalyserData::highFftBufferSize>(
		data.highPreCompBuffer, data.highPreCompPos, highPreCompBuf);

	AnalyserData::copyBuffer<AnalyserData::lowFftSize, AnalyserData::lowFftBufferSize>(data.lowPreSatBuffer,
																					   data.lowPreSatPos, lowFFTBuffer);
	AnalyserData::copyBuffer<AnalyserData::midFftSize, AnalyserData::midFftBufferSize>(data.midPreSatBuffer,
																					   data.midPreSatPos, midFFTBuffer);
	AnalyserData::copyBuffer<AnalyserData::highFftSize, AnalyserData::highFftBufferSize>(
		data.highPreSatBuffer, data.highPreSatPos, highFFTBuffer);

	AnalyserData::copyBuffer<AnalyserData::lowFftSize, AnalyserData::lowFftBufferSize>(data.lowPreSatBuffer,
																					   data.lowPreSatPos, lowPreSat);
	AnalyserData::copyBuffer<AnalyserData::lowFftSize, AnalyserData::lowFftBufferSize>(data.lowPostSatBuffer,
																					   data.lowPostSatPos, lowPostSat);
	AnalyserData::copyBuffer<AnalyserData::midFftSize, AnalyserData::midFftBufferSize>(data.midPreSatBuffer,
																					   data.midPreSatPos, midPreSat);
	AnalyserData::copyBuffer<AnalyserData::midFftSize, AnalyserData::midFftBufferSize>(data.midPostSatBuffer,
																					   data.midPostSatPos, midPostSat);
	AnalyserData::copyBuffer<AnalyserData::highFftSize, AnalyserData::highFftBufferSize>(
		data.highPreSatBuffer, data.highPreSatPos, highPreSat);
	AnalyserData::copyBuffer<AnalyserData::highFftSize, AnalyserData::highFftBufferSize>(
		data.highPostSatBuffer, data.highPostSatPos, highPostSat);

	auto doFFT = [](juce::dsp::FFT& fft, float* buffer, int fftSize, float* spectrum, int numBins)
	{
		for (int i = 0; i < fftSize; ++i)
		{
			const float hanning =
				0.5f * (1.0f - std::cos(2.0f * juce::MathConstants<float>::pi * i / static_cast<float>(fftSize - 1)));
			buffer[i] *= hanning;
		}

		fft.performRealOnlyForwardTransform(buffer, false);

		const float normFactor = 2.0f / static_cast<float>(fftSize);

		for (int bin = 0; bin < numBins; ++bin)
		{
			const float re = buffer[2 * bin];
			const float im = buffer[2 * bin + 1];
			const float magnitude = normFactor * std::sqrt(re * re + im * im);
			spectrum[bin] = juce::Decibels::gainToDecibels(magnitude);
		}
	};

	doFFT(*lowFft, lowPreCompBuf.data(), AnalyserData::lowFftSize, lowPreCompSpectrum.data(), AnalyserData::lowNumBins);
	doFFT(*midFft, midPreCompBuf.data(), AnalyserData::midFftSize, midPreCompSpectrum.data(), AnalyserData::midNumBins);
	doFFT(*highFft, highPreCompBuf.data(), AnalyserData::highFftSize, highPreCompSpectrum.data(),
		  AnalyserData::highNumBins);

	doFFT(*lowFft, lowFFTBuffer.data(), AnalyserData::lowFftSize, lowSpectrum.data(), AnalyserData::lowNumBins);
	doFFT(*midFft, midFFTBuffer.data(), AnalyserData::midFftSize, midSpectrum.data(), AnalyserData::midNumBins);
	doFFT(*highFft, highFFTBuffer.data(), AnalyserData::highFftSize, highSpectrum.data(), AnalyserData::highNumBins);

	auto computeHarmonics = [&doFFT](juce::dsp::FFT& fft, int fftSize, int numBins, std::vector<float>& preBuf,
									 std::vector<float>& postBuf, std::vector<HarmonicPoint>& harmonics)
	{
		doFFT(fft, preBuf.data(), fftSize, preBuf.data(), numBins);
		doFFT(fft, postBuf.data(), fftSize, postBuf.data(), numBins);

		harmonics.clear();
		harmonics.reserve(static_cast<size_t>(numBins));

		constexpr float fundamentalThresholdDb = -60.0f;
		int fundamentalBin = -1;

		for (size_t bin = 1; bin < static_cast<size_t>(numBins); ++bin)
		{
			const float preDb = preBuf[bin];
			if (preDb > fundamentalThresholdDb)
			{
				fundamentalBin = static_cast<int>(bin);
				break;
			}
		}

		for (int bin = 1; bin < numBins; ++bin)
		{
			const float preMag = juce::Decibels::decibelsToGain(preBuf[static_cast<size_t>(bin)]);
			const float postMag = juce::Decibels::decibelsToGain(postBuf[static_cast<size_t>(bin)]);

			if (postMag > preMag * 1.05f)
			{
				const float postDb = postBuf[static_cast<size_t>(bin)];
				bool isOdd = true;

				if (fundamentalBin > 0)
				{
					const float ratio = static_cast<float>(bin) / static_cast<float>(fundamentalBin);
					const int harmonicOrder = static_cast<int>(std::round(ratio));
					if (std::abs(bin - fundamentalBin * harmonicOrder) <= 1 && harmonicOrder >= 2)
						isOdd = (harmonicOrder % 2 == 1);
				}

				harmonics.push_back({static_cast<float>(bin), postDb, isOdd});
			}
		}
	};

	{
		std::vector<float> lPre(lowPreSat.begin(), lowPreSat.begin() + AnalyserData::lowFftBufferSize);
		std::vector<float> lPost(lowPostSat.begin(), lowPostSat.begin() + AnalyserData::lowFftBufferSize);
		computeHarmonics(*lowFft, AnalyserData::lowFftSize, AnalyserData::lowNumBins, lPre, lPost, lowHarmonics);

		std::vector<float> mPre(midPreSat.begin(), midPreSat.begin() + AnalyserData::midFftBufferSize);
		std::vector<float> mPost(midPostSat.begin(), midPostSat.begin() + AnalyserData::midFftBufferSize);
		computeHarmonics(*midFft, AnalyserData::midFftSize, AnalyserData::midNumBins, mPre, mPost, midHarmonics);

		std::vector<float> hPre(highPreSat.begin(), highPreSat.begin() + AnalyserData::highFftBufferSize);
		std::vector<float> hPost(highPostSat.begin(), highPostSat.begin() + AnalyserData::highFftBufferSize);
		computeHarmonics(*highFft, AnalyserData::highFftSize, AnalyserData::highNumBins, hPre, hPost, highHarmonics);
	}

	lowPostComp = data.lowPostCompLevel.load(std::memory_order_acquire);
	midPostComp = data.midPostCompLevel.load(std::memory_order_acquire);
	highPostComp = data.highPostCompLevel.load(std::memory_order_acquire);
	lowCurrentLevel = data.lowLevel.load(std::memory_order_acquire);
	midCurrentLevel = data.midLevel.load(std::memory_order_acquire);
	highCurrentLevel = data.highLevel.load(std::memory_order_acquire);
	limGainReduction = data.limiterGR.load(std::memory_order_acquire);

	repaint();
}

//==============================================================================
//  Coordinate mapping helpers
//==============================================================================
float AudioAnalyser::binToX(int bin, int numBins, float width, double sampleRate) const noexcept
{
	if (bin <= 0)
		return 0.0f;

	constexpr float minFreq = 20.0f;
	constexpr float maxFreq = 20000.0f;

	const float freq = static_cast<float>(bin) / static_cast<float>(numBins) * static_cast<float>(sampleRate) * 0.5f;

	const float norm = std::log(freq / minFreq) / std::log(maxFreq / minFreq);
	return juce::jlimit(0.0f, 1.0f, norm) * width;
}

float AudioAnalyser::dbToY(float db, float height) const noexcept
{
	const float norm = (db - minDb) / (maxDb - minDb);
	return juce::jmap(norm, height, 0.0f);
}

//==============================================================================
//  Paint
//==============================================================================
void AudioAnalyser::paint(juce::Graphics& g)
{
	const auto localBounds = getLocalBounds().toFloat();

	// Narrow the display horizontally — adjust the x reduction to taste
	const auto bounds = localBounds.reduced(2.0f, 0.0f);

	constexpr float cornerRadius = 6.0f;

	// --- Background with rounded corners ----------------------------------
	g.setColour(juce::Colour(0xFF080808));
	g.fillRoundedRectangle(bounds, cornerRadius);

	// Clip all subsequent drawing to the rounded rect so nothing bleeds out
	juce::Path clipPath;
	clipPath.addRoundedRectangle(bounds, cornerRadius);
	g.reduceClipRegion(clipPath);

	// --- Grid lines -------------------------------------------------------
	g.setColour(juce::Colour(0x22FFFFFF));
	constexpr float gridDb[] = {-60.0f, -40.0f, -20.0f, 0.0f};
	for (float db : gridDb)
	{
		const float y = dbToY(db, bounds.getHeight());
		g.drawHorizontalLine(static_cast<int>(bounds.getY() + y), bounds.getX(), bounds.getRight());
	}

	juce::Colour bandColArray[3] = {bandColours.low, bandColours.mid, bandColours.high};

	// Layer 1: Pre-compressor spectrum (dimmed background)
	drawSpectrum(g, bounds, lowPreCompSpectrum, bandColArray[0].withMultipliedBrightness(0.5f), 1.0f);
	drawSpectrum(g, bounds, midPreCompSpectrum, bandColArray[1].withMultipliedBrightness(0.5f), 1.0f);
	drawSpectrum(g, bounds, highPreCompSpectrum, bandColArray[2].withMultipliedBrightness(0.5f), 1.0f);

	// Layer 2: Post-compressor spectrum (bright foreground)
	drawSpectrum(g, bounds, lowSpectrum, bandColArray[0], 1.5f);
	drawSpectrum(g, bounds, midSpectrum, bandColArray[1], 1.5f);
	drawSpectrum(g, bounds, highSpectrum, bandColArray[2], 1.5f);

	// Layer 3: Current level lines (1/3 width each)
	drawCurrentLevelLine(g, bounds, lowCurrentLevel, bandColArray[0], 0);
	drawCurrentLevelLine(g, bounds, midCurrentLevel, bandColArray[1], 1);
	drawCurrentLevelLine(g, bounds, highCurrentLevel, bandColArray[2], 2);

	// Layer 4: Harmonic scatter
	juce::Colour evenColours[3] = {juce::Colour(0xFFCC44CC), juce::Colour(0xFF4488FF), juce::Colour(0xFF8844FF)};
	drawHarmonicScatter(g, bounds, lowHarmonics, bandColArray[0], evenColours[0], AnalyserData::lowFftSize);
	drawHarmonicScatter(g, bounds, midHarmonics, bandColArray[1], evenColours[1], AnalyserData::midFftSize);
	drawHarmonicScatter(g, bounds, highHarmonics, bandColArray[2], evenColours[2], AnalyserData::highFftSize);

	// Layer 5: Limiter GR
	drawLimiterGR(g, bounds, limGainReduction);

	// --- Rounded border on top -------------------------------------------
	g.setColour(juce::Colour(0x44FFFFFF));
	g.drawRoundedRectangle(bounds, cornerRadius, 1.0f);
}

void AudioAnalyser::resized() {}

//==============================================================================
//  Drawing helpers
//==============================================================================
// Note: drawBackground removed — background is now drawn directly in paint()
// with rounded corners and clipping set up before any other drawing.

template <size_t NumBins>
void AudioAnalyser::drawSpectrum(juce::Graphics& g, juce::Rectangle<float> bounds,
								 const std::array<float, NumBins>& spectrum, juce::Colour colour, float lineThickness)
{
	if (bounds.getWidth() <= 0 || bounds.getHeight() <= 0)
		return;

	const float width = bounds.getWidth();
	const float height = bounds.getHeight();

	juce::Path path;
	bool first = true;

	for (size_t bin = 0; bin < NumBins; ++bin)
	{
		const float x =
			bounds.getX() + binToX(static_cast<int>(bin), static_cast<int>(NumBins), width, currentSampleRate);
		const float y = bounds.getY() + dbToY(spectrum[bin], height);

		if (first)
		{
			path.startNewSubPath(x, y);
			first = false;
		}
		else
		{
			path.lineTo(x, y);
		}
	}

	juce::Path fillPath(path);
	fillPath.lineTo(bounds.getRight(), bounds.getY() + dbToY(-80.0f, height));
	fillPath.lineTo(bounds.getX(), bounds.getY() + dbToY(-80.0f, height));
	fillPath.closeSubPath();

	g.setColour(colour.withMultipliedAlpha(0.15f));
	g.fillPath(fillPath);

	g.setColour(colour);
	g.strokePath(path, juce::PathStrokeType(lineThickness));
}

void AudioAnalyser::drawHarmonicScatter(juce::Graphics& g, juce::Rectangle<float> bounds,
										const std::vector<HarmonicPoint>& harmonics, juce::Colour oddColour,
										juce::Colour evenColour, int fftSize)
{
	if (harmonics.empty())
		return;

	const float width = bounds.getWidth();
	const float height = bounds.getHeight();
	const int numBins = fftSize / 2;

	for (const auto& h : harmonics)
	{
		const float x = bounds.getX() + binToX(static_cast<int>(h.bin), numBins, width, currentSampleRate);
		const float y = bounds.getY() + dbToY(h.levelDb, height);

		const float norm = juce::jlimit(0.0f, 1.0f, (h.levelDb - minDb) / (maxDb - minDb));
		const float radius = 1.5f + norm * 2.5f;

		g.setColour(h.isOdd ? oddColour : evenColour);
		g.fillEllipse(x - radius, y - radius, radius * 2.0f, radius * 2.0f);
	}
}

void AudioAnalyser::drawCurrentLevelLine(juce::Graphics& g, juce::Rectangle<float> bounds, float levelDb,
										 juce::Colour colour, int bandIndex)
{
	if (levelDb <= minDb)
		return;

	const float height = bounds.getHeight();
	const float y = bounds.getY() + dbToY(levelDb, height);
	const float thirdWidth = bounds.getWidth() / 3.0f;
	const float xStart = bounds.getX() + bandIndex * thirdWidth;
	const float xEnd = xStart + thirdWidth;

	g.setColour(colour.withMultipliedAlpha(0.9f));
	g.drawHorizontalLine(static_cast<int>(y), xStart, xEnd);
}

void AudioAnalyser::drawLimiterGR(juce::Graphics& g, juce::Rectangle<float> bounds, float grDb)
{
	const float width = bounds.getWidth();
	const float height = bounds.getHeight();

	const juce::Colour limColour = juce::Colour(0xFF6633CC);

	const float lineDb = grDb; // grDb is negative, e.g. -3.0
	const float y = bounds.getY() + dbToY(lineDb, height);

	// Fill from top of display down to the line
	g.setColour(limColour.withMultipliedAlpha(0.3f));
	g.fillRect(bounds.getX(), bounds.getY(), width, y - bounds.getY());

	g.setColour(limColour);
	g.drawHorizontalLine(static_cast<int>(y), bounds.getX(), bounds.getRight());
}