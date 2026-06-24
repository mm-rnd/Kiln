#pragma once

#include <array>
#include <atomic>
#include <juce_dsp/juce_dsp.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <memory>
#include <vector>

//==============================================================================
/**
 * Thread-safe data container shared between the audio processor (writer)
 * and the analyser UI component (reader).
 *
 * Each band has its own FFT order so that the low band gets higher frequency
 * resolution while the high band gets better time resolution.
 *
 * The audio thread writes samples into ring buffers and updates atomic
 * level/GR values. The UI thread reads them on a 30 Hz timer.
 */
struct AnalyserData
{
	// Per-band FFT orders
	static constexpr int lowFftOrder = 13;	// 8192 points — ~5 Hz/bin at 44100, dense low-end
	static constexpr int midFftOrder = 11;	// 2048 points — balanced
	static constexpr int highFftOrder = 10; // 1024 points — better time resolution for highs

	static constexpr int lowFftSize = 1 << lowFftOrder;
	static constexpr int midFftSize = 1 << midFftOrder;
	static constexpr int highFftSize = 1 << highFftOrder;

	static constexpr int lowNumBins = lowFftSize / 2;
	static constexpr int midNumBins = midFftSize / 2;
	static constexpr int highNumBins = highFftSize / 2;

	// performRealOnlyForwardTransform needs 2*fftSize elements
	static constexpr int lowFftBufferSize = 2 * lowFftSize;
	static constexpr int midFftBufferSize = 2 * midFftSize;
	static constexpr int highFftBufferSize = 2 * highFftSize;

	// --- Ring buffers for final (post-gain) level display -------------------
	std::array<float, lowFftSize> lowFinalBuffer{};
	std::array<float, midFftSize> midFinalBuffer{};
	std::array<float, highFftSize> highFinalBuffer{};
	std::atomic<int> lowFinalPos{0};
	std::atomic<int> midFinalPos{0};
	std::atomic<int> highFinalPos{0};

	// --- Pre-saturation (post-compressor) buffers for spectrum -------------
	std::array<float, lowFftSize> lowPreSatBuffer{};
	std::array<float, midFftSize> midPreSatBuffer{};
	std::array<float, highFftSize> highPreSatBuffer{};
	std::atomic<int> lowPreSatPos{0};
	std::atomic<int> midPreSatPos{0};
	std::atomic<int> highPreSatPos{0};

	// --- Pre-compressor (post-crossover) buffers for spectrum --------------
	std::array<float, lowFftSize> lowPreCompBuffer{};
	std::array<float, midFftSize> midPreCompBuffer{};
	std::array<float, highFftSize> highPreCompBuffer{};
	std::atomic<int> lowPreCompPos{0};
	std::atomic<int> midPreCompPos{0};
	std::atomic<int> highPreCompPos{0};

	// --- Post-saturation buffers (for harmonic analysis) -------------------
	std::array<float, lowFftSize> lowPostSatBuffer{};
	std::array<float, midFftSize> midPostSatBuffer{};
	std::array<float, highFftSize> highPostSatBuffer{};
	std::atomic<int> lowPostSatPos{0};
	std::atomic<int> midPostSatPos{0};
	std::atomic<int> highPostSatPos{0};

	// --- Post-compressor RMS level (dB) ------------------------------------
	std::atomic<float> lowPostCompLevel{-100.0f};
	std::atomic<float> midPostCompLevel{-100.0f};
	std::atomic<float> highPostCompLevel{-100.0f};

	// --- Limiter gain reduction (dB) ---------------------------------------
	std::atomic<float> limiterGR{0.0f};

	// --- Current RMS level per band (dB) -----------------------------------
	std::atomic<float> lowLevel{-100.0f};
	std::atomic<float> midLevel{-100.0f};
	std::atomic<float> highLevel{-100.0f};

	// --- Saturation harmonic energy (dB, for scatter) ----------------------
	std::atomic<float> lowSatEnergy{-100.0f};
	std::atomic<float> midSatEnergy{-100.0f};
	std::atomic<float> highSatEnergy{-100.0f};

	// --- Sample rate (written by audio thread on prepareToPlay) ------------
	std::atomic<double> sampleRate{44100.0};

	// --- Helpers for audio thread ------------------------------------------

	/** Push a sample into a ring buffer (lock-free, single writer). */
	template <size_t Size>
	static void pushSample(std::array<float, Size>& buffer, std::atomic<int>& writePos, float sample) noexcept
	{
		const int pos = writePos.load(std::memory_order_relaxed);
		buffer[static_cast<size_t>(pos)] = sample;
		writePos.store((pos + 1) % static_cast<int>(Size), std::memory_order_release);
	}

	template <size_t RingSize, size_t BufSize>
	static void copyBuffer(const std::array<float, RingSize>& src, std::atomic<int>& writePos,
						   std::array<float, BufSize>& dst) noexcept
	{
		static_assert(BufSize >= RingSize, "dst must be at least as large as src");
		const int pos = writePos.load(std::memory_order_acquire);
		for (size_t i = 0; i < RingSize; ++i)
		{
			const size_t idx = (static_cast<size_t>(pos) + i) % RingSize;
			dst[i] = src[idx];
		}
		for (size_t i = RingSize; i < BufSize; ++i)
			dst[i] = 0.0f;
	}

	template <size_t Size>
	static float computeRMS(const std::array<float, Size>& buffer, std::atomic<int>& writePos) noexcept
	{
		const int pos = writePos.load(std::memory_order_acquire);
		float sumSq = 0.0f;
		for (size_t i = 0; i < Size; ++i)
		{
			const size_t idx = (static_cast<size_t>(pos) + i) % Size;
			const float sample = buffer[idx];
			sumSq += sample * sample;
		}
		const float rms = std::sqrt(sumSq / static_cast<float>(Size));
		return juce::Decibels::gainToDecibels(rms);
	}
};

//==============================================================================
/**
 * Audio analyser component that renders a multi-layer spectrum display.
 *
 * Each band uses a different FFT size for optimal frequency/time resolution.
 * The sample rate is read from AnalyserData so binToX is always accurate.
 *
 * Layers (back to front):
 *   1. Pre-compressor per-band FFT spectrum (dimmed background)
 *   2. Post-compressor per-band FFT spectrum (bright foreground)
 *   3. Saturation harmonics scatter plot
 *   4. Current (post-gain) level line per band (1/3 width each)
 *   5. Limiter gain reduction (purple line + opaque fill above)
 */
class AudioAnalyser final : public juce::Component, private juce::Timer
{
  public:
	/** Band colour configuration. */
	struct BandColours
	{
		juce::Colour low;
		juce::Colour mid;
		juce::Colour high;
	};

	explicit AudioAnalyser(AnalyserData& data, BandColours colours);
	~AudioAnalyser() override;

	void paint(juce::Graphics& g) override;
	void resized() override;

	/** Start/stop the refresh timer. */
	void startAnalyser();
	void stopAnalyser();

  private:
	AnalyserData& data;
	BandColours bandColours;

	// Per-band FFT processors
	std::unique_ptr<juce::dsp::FFT> lowFft;
	std::unique_ptr<juce::dsp::FFT> midFft;
	std::unique_ptr<juce::dsp::FFT> highFft;

	// Per-band FFT working buffers (2x FFT size for in-place transform)
	std::array<float, AnalyserData::lowFftBufferSize> lowFFTBuffer{};
	std::array<float, AnalyserData::midFftBufferSize> midFFTBuffer{};
	std::array<float, AnalyserData::highFftBufferSize> highFFTBuffer{};

	// Pre-compressor working buffers
	std::array<float, AnalyserData::lowFftBufferSize> lowPreCompBuf{};
	std::array<float, AnalyserData::midFftBufferSize> midPreCompBuf{};
	std::array<float, AnalyserData::highFftBufferSize> highPreCompBuf{};

	// Pre/post saturation buffers for harmonic analysis
	std::array<float, AnalyserData::lowFftBufferSize> lowPreSat{};
	std::array<float, AnalyserData::lowFftBufferSize> lowPostSat{};
	std::array<float, AnalyserData::midFftBufferSize> midPreSat{};
	std::array<float, AnalyserData::midFftBufferSize> midPostSat{};
	std::array<float, AnalyserData::highFftBufferSize> highPreSat{};
	std::array<float, AnalyserData::highFftBufferSize> highPostSat{};

	// Spectrum magnitude arrays (one entry per bin)
	std::array<float, AnalyserData::lowNumBins> lowSpectrum{};
	std::array<float, AnalyserData::midNumBins> midSpectrum{};
	std::array<float, AnalyserData::highNumBins> highSpectrum{};

	std::array<float, AnalyserData::lowNumBins> lowPreCompSpectrum{};
	std::array<float, AnalyserData::midNumBins> midPreCompSpectrum{};
	std::array<float, AnalyserData::highNumBins> highPreCompSpectrum{};

	// Harmonic scatter data
	struct HarmonicPoint
	{
		float bin;
		float levelDb;
		bool isOdd;
	};
	std::vector<HarmonicPoint> lowHarmonics;
	std::vector<HarmonicPoint> midHarmonics;
	std::vector<HarmonicPoint> highHarmonics;

	// Cached values from the last timer tick
	float lowPostComp = -100.0f;
	float midPostComp = -100.0f;
	float highPostComp = -100.0f;
	float lowCurrentLevel = -100.0f;
	float midCurrentLevel = -100.0f;
	float highCurrentLevel = -100.0f;
	float limGainReduction = 0.0f;
	double currentSampleRate = 44100.0;

	// Timer callback — refresh at ~30 fps
	void timerCallback() override;

	// --- Drawing helpers ---------------------------------------------------
	void drawBackground(juce::Graphics& g, juce::Rectangle<float> bounds);

	template <size_t NumBins>
	void drawSpectrum(juce::Graphics& g, juce::Rectangle<float> bounds, const std::array<float, NumBins>& spectrum,
					  juce::Colour colour, float lineThickness);

	void drawHarmonicScatter(juce::Graphics& g, juce::Rectangle<float> bounds,
							 const std::vector<HarmonicPoint>& harmonics, juce::Colour oddColour,
							 juce::Colour evenColour, int fftSize);

	void drawCurrentLevelLine(juce::Graphics& g, juce::Rectangle<float> bounds, float levelDb, juce::Colour colour,
							  int bandIndex);
	void drawLimiterGR(juce::Graphics& g, juce::Rectangle<float> bounds, float grDb);

	/** Map a frequency bin to an x-coordinate (log scale).
	 *  fftSize and sampleRate are passed explicitly to support per-band FFTs. */
	float binToX(int bin, int numBins, float width, double sampleRate) const noexcept;

	/** Map a dB level to a y-coordinate (0 = top, height = bottom). */
	float dbToY(float db, float height) const noexcept;

	static constexpr float minDb = -80.0f;
	static constexpr float maxDb = 0.0f;
	static constexpr float grMaxDb = 24.0f;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioAnalyser)
};
