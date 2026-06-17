#include "LinearPhaseCrossover.h"
#include <algorithm>
#include <cmath>
#include <cstring>

// ---------------------------------------------------------------------------
//  Preparation
// ---------------------------------------------------------------------------

void LinearPhaseCrossover::prepare(double sr, int bs)
{
	sampleRate = sr;
	blockSize = bs;

	// (Re-)initialise the smoothers, preserving any target that was set
	// before the first prepare() call.
	const float lo = smoothedLowCrossover.getTargetValue();
	const float hi = smoothedHighCrossover.getTargetValue();
	smoothedLowCrossover.reset(sr, 0.05); // 50 ms ramp
	smoothedHighCrossover.reset(sr, 0.05);
	smoothedLowCrossover.setCurrentAndTargetValue(lo);
	smoothedHighCrossover.setCurrentAndTargetValue(hi);

	lowCrossoverHz = lo;
	highCrossoverHz = hi;

	designFilters();
	ensureBuffers(numChannels > 0 ? numChannels : 2);
	prepareConvolutions();
}

// ---------------------------------------------------------------------------
//  Crossover setters – redesign FIR when a frequency changes
// ---------------------------------------------------------------------------

void LinearPhaseCrossover::setLowCrossover(float frequencyHz)
{
	smoothedLowCrossover.setTargetValue(frequencyHz);
}

void LinearPhaseCrossover::setHighCrossover(float frequencyHz)
{
	smoothedHighCrossover.setTargetValue(frequencyHz);
}

// ---------------------------------------------------------------------------
//  Getters
// ---------------------------------------------------------------------------

float LinearPhaseCrossover::getLowCrossover() const noexcept
{
	return smoothedLowCrossover.getTargetValue();
}
float LinearPhaseCrossover::getHighCrossover() const noexcept
{
	return smoothedHighCrossover.getTargetValue();
}

[[nodiscard]] double LinearPhaseCrossover::getSampleRate() const noexcept
{
	return sampleRate;
}

bool LinearPhaseCrossover::hasDelay() const noexcept
{
	return true;
}

int LinearPhaseCrossover::getGroupDelaySamples() const noexcept
{
	const int numTaps = static_cast<int>(lowCoeffs.size());
	return (numTaps > 0) ? (numTaps - 1) / 2 : 0;
}

// ---------------------------------------------------------------------------
//  Band accessors
// ---------------------------------------------------------------------------

const float* LinearPhaseCrossover::getLowBand(int ch) const
{
	return lowBand[static_cast<size_t>(ch)].data();
}
const float* LinearPhaseCrossover::getMidBand(int ch) const
{
	return midBand[static_cast<size_t>(ch)].data();
}
const float* LinearPhaseCrossover::getHighBand(int ch) const
{
	return highBand[static_cast<size_t>(ch)].data();
}

// ---------------------------------------------------------------------------
//  Modified Bessel function I0  (full range)
// ---------------------------------------------------------------------------

float LinearPhaseCrossover::besselI0(float x)
{
	const float ax = std::abs(x);
	const float y = x / 3.75f;

	if (ax <= 3.75f)
	{
		const float t = y * y;
		return 1.0f + t * (3.5156229f +
						   t * (3.0899424f + t * (1.2067492f + t * (0.2659732f + t * (0.0360768f + t * 0.0045813f)))));
	}

	return std::exp(ax) / std::sqrt(2.0f * juce::MathConstants<float>::pi * ax) *
		   (1.0f + 1.0f / (8.0f * ax) + 9.0f / (128.0f * ax * ax));
}

// ---------------------------------------------------------------------------
//  Windowed-sinc lowpass FIR design  (Kaiser window)
// ---------------------------------------------------------------------------

std::vector<float> LinearPhaseCrossover::designLowpass(float cutoffHz, double sr)
{
	const float transWidthHz = cutoffHz * 0.2f;
	const float deltaDb = 80.0f;
	const float beta = 0.1102f * (deltaDb - 8.7f);
	const int order = static_cast<int>(
		std::ceil((deltaDb - 7.95f) / (2.285f * juce::MathConstants<float>::twoPi * transWidthHz / sr)));
	const int numTaps = (order % 2 == 0) ? order + 1 : order;
	const int half = (numTaps - 1) / 2;

	std::vector<float> h(static_cast<size_t>(numTaps), 0.0f);
	const float normCutoff = cutoffHz / static_cast<float>(sr);

	for (int i = 0; i < numTaps; ++i)
	{
		const float x = static_cast<float>(i - half) * normCutoff;
		h[static_cast<size_t>(i)] = (std::abs(x) < 1e-12f)
										? 2.0f * normCutoff
										: 2.0f * normCutoff * std::sin(juce::MathConstants<float>::pi * x) /
											  (juce::MathConstants<float>::pi * x);
	}

	const float alpha = static_cast<float>(half);
	const float i0Beta = besselI0(beta);

	for (int i = 0; i < numTaps; ++i)
	{
		const float r = static_cast<float>(i - half) / alpha;
		if (std::abs(r) < 1.0f)
		{
			const float w = besselI0(beta * std::sqrt(1.0f - r * r)) / i0Beta;
			h[static_cast<size_t>(i)] *= w;
		}
		else
		{
			h[static_cast<size_t>(i)] = 0.0f;
		}
	}

	float sum = 0.0f;
	for (float v : h)
		sum += v;
	if (sum != 0.0f)
		for (float& v : h)
			v /= sum;

	return h;
}

// ---------------------------------------------------------------------------
//  Filter design helpers
// ---------------------------------------------------------------------------

void LinearPhaseCrossover::designFilters()
{
	const float nyquist = static_cast<float>(sampleRate * 0.5);
	const float lo = juce::jlimit(1.0f, nyquist - 1.0f, lowCrossoverHz);
	const float hi = juce::jlimit(lo + 1.0f, nyquist - 1.0f, highCrossoverHz);

	lowCoeffs = designLowpass(lo, sampleRate);
	highCoeffs = designLowpass(hi, sampleRate);

	// Both filters must have the same length so the mid-band subtraction
	// (lowpass_high − lowpass_low) is properly aligned.
	const int maxLen = std::max(static_cast<int>(lowCoeffs.size()), static_cast<int>(highCoeffs.size()));
	auto padToLength = [](std::vector<float>& v, int targetLen)
	{
		const int cur = static_cast<int>(v.size());
		if (cur < targetLen)
		{
			const int pad = targetLen - cur;
			const int padLeft = pad / 2;
			const int padRight = pad - padLeft;
			v.insert(v.begin(), static_cast<size_t>(padLeft), 0.0f);
			v.insert(v.end(), static_cast<size_t>(padRight), 0.0f);
		}
	};
	padToLength(lowCoeffs, maxLen);
	padToLength(highCoeffs, maxLen);

	// Update group delay
	delayLength = (maxLen - 1) / 2;

	// Load the new impulse responses into the convolution engines.
	// loadImpulseResponse is wait-free (posts to a background thread).
	loadImpulseResponses();

	// If we already know the channel count (i.e. prepare() was called),
	// synchronously install the new IRs so they are active for the very
	// next split() call.  Convolution::prepare() drains the pending
	// message queue, builds the new engine, and crossfades to it.
	if (numChannels > 0)
		prepareConvolutions();

	// Resize delay states if the group delay changed
	for (auto& ds : delayStates)
	{
		if (static_cast<int>(ds.buffer.size()) != delayLength)
		{
			ds.buffer.assign(static_cast<size_t>(delayLength), 0.0f);
			ds.writePos = 0;
		}
	}
}

// ---------------------------------------------------------------------------
//  Load FIR coefficients as impulse responses into the convolution engines
// ---------------------------------------------------------------------------

void LinearPhaseCrossover::loadImpulseResponses()
{
	const int numTaps = static_cast<int>(lowCoeffs.size());

	// Low-crossover lowpass IR
	juce::AudioBuffer<float> lowIR(1, numTaps);
	lowIR.copyFrom(0, 0, lowCoeffs.data(), numTaps);
	lowConvolution.loadImpulseResponse(std::move(lowIR), sampleRate, juce::dsp::Convolution::Stereo::no,
									   juce::dsp::Convolution::Trim::no, juce::dsp::Convolution::Normalise::no);

	// High-crossover lowpass IR
	juce::AudioBuffer<float> highIR(1, numTaps);
	highIR.copyFrom(0, 0, highCoeffs.data(), numTaps);
	highConvolution.loadImpulseResponse(std::move(highIR), sampleRate, juce::dsp::Convolution::Stereo::no,
										juce::dsp::Convolution::Trim::no, juce::dsp::Convolution::Normalise::no);
}

// ---------------------------------------------------------------------------
//  Prepare the convolution engines with the current ProcessSpec
// ---------------------------------------------------------------------------

void LinearPhaseCrossover::prepareConvolutions()
{
	juce::dsp::ProcessSpec spec;
	spec.sampleRate = sampleRate;
	spec.maximumBlockSize = static_cast<juce::uint32>(blockSize);
	spec.numChannels = static_cast<juce::uint32>(numChannels);

	lowConvolution.prepare(spec);
	highConvolution.prepare(spec);
}

// ---------------------------------------------------------------------------
//  Buffer management
// ---------------------------------------------------------------------------

void LinearPhaseCrossover::ensureBuffers(int channels)
{
	numChannels = channels;
	const auto len = static_cast<size_t>(blockSize);

	// Convolution output buffers
	lowConvOutput.setSize(channels, blockSize, false, false, true);
	highConvOutput.setSize(channels, blockSize, false, false, true);

	// Delay states (circular buffer per channel)
	delayStates.resize(static_cast<size_t>(channels));
	for (int ch = 0; ch < channels; ++ch)
	{
		auto& ds = delayStates[static_cast<size_t>(ch)];
		ds.buffer.assign(static_cast<size_t>(delayLength), 0.0f);
		ds.writePos = 0;
	}

	// Output band buffers
	lowBand.resize(static_cast<size_t>(channels));
	midBand.resize(static_cast<size_t>(channels));
	highBand.resize(static_cast<size_t>(channels));

	for (int ch = 0; ch < channels; ++ch)
	{
		lowBand[static_cast<size_t>(ch)].assign(len, 0.0f);
		midBand[static_cast<size_t>(ch)].assign(len, 0.0f);
		highBand[static_cast<size_t>(ch)].assign(len, 0.0f);
	}
}

// ---------------------------------------------------------------------------
//  Split into three bands  (FFT-based convolution via juce::dsp::Convolution)
// ---------------------------------------------------------------------------

void LinearPhaseCrossover::split(juce::AudioBuffer<float>& buffer)
{
	const int numSamples = buffer.getNumSamples();
	const int channels = buffer.getNumChannels();

	// Handle channel count change (full re-init including convolution
	// re-prepare).
	if (channels != numChannels)
	{
		ensureBuffers(channels);
		prepareConvolutions();
	}

	// Advance crossover smoothers and, if the frequency has changed,
	// redesign the filters for this block.
	if (smoothedLowCrossover.isSmoothing() || smoothedHighCrossover.isSmoothing())
	{
		const float newLow = smoothedLowCrossover.skip(numSamples);
		const float newHigh = smoothedHighCrossover.skip(numSamples);

		if (!floatMatch(newLow, lowCrossoverHz) || !floatMatch(newHigh, highCrossoverHz))
		{
			lowCrossoverHz = newLow;
			highCrossoverHz = newHigh;
			designFilters();
		}
	}

	// Handle block size change: resize output buffers.
	if (numSamples != blockSize)
	{
		blockSize = numSamples;
		ensureBuffers(numChannels);
	}

	// Clear convolution output buffers so the engines accumulate from zero.
	lowConvOutput.clear(0, numSamples);
	highConvOutput.clear(0, numSamples);

	// Build a const input block from the original buffer.
	const juce::dsp::AudioBlock<const float> inputBlock(buffer.getArrayOfReadPointers(), static_cast<size_t>(channels),
														static_cast<size_t>(numSamples));

	// Process low-crossover lowpass convolution (non-replacing).
	{
		juce::dsp::AudioBlock<float> outputBlock(lowConvOutput);
		juce::dsp::ProcessContextNonReplacing<float> context(inputBlock, outputBlock);
		lowConvolution.process(context);
	}

	// Process high-crossover lowpass convolution (non-replacing).
	{
		juce::dsp::AudioBlock<float> outputBlock(highConvOutput);
		juce::dsp::ProcessContextNonReplacing<float> context(inputBlock, outputBlock);
		highConvolution.process(context);
	}

	// Derive the three bands.
	//   low  = lowpass@lowCrossover
	//   high = delayed_original − lowpass@highCrossover
	//   mid  = lowpass@highCrossover − lowpass@lowCrossover
	const int gd = delayLength;

	for (int ch = 0; ch < channels; ++ch)
	{
		const float* in = buffer.getReadPointer(ch);
		const float* lo = lowConvOutput.getReadPointer(ch);
		const float* hp = highConvOutput.getReadPointer(ch);
		auto* loOut = lowBand[static_cast<size_t>(ch)].data();
		auto* mdOut = midBand[static_cast<size_t>(ch)].data();
		auto* hiOut = highBand[static_cast<size_t>(ch)].data();

		auto& ds = delayStates[static_cast<size_t>(ch)];

		if (gd > 0)
		{
			for (int n = 0; n < numSamples; ++n)
			{
				// Read the oldest sample in the circular buffer (= delayed input)
				const float delayed = ds.buffer[static_cast<size_t>(ds.writePos)];

				// Overwrite with the current input sample
				ds.buffer[static_cast<size_t>(ds.writePos)] = in[n];
				ds.writePos = (ds.writePos + 1) % gd;

				loOut[n] = lo[n];
				hiOut[n] = delayed - hp[n];
				mdOut[n] = hp[n] - lo[n];
			}
		}
		else
		{
			// Zero group delay (degenerate single-tap filter)
			for (int n = 0; n < numSamples; ++n)
			{
				loOut[n] = lo[n];
				hiOut[n] = in[n] - hp[n];
				mdOut[n] = hp[n] - lo[n];
			}
		}
	}
}

bool LinearPhaseCrossover::floatMatch(float a, float b)
{
	return std::fabs(a - b) <= floatEpsilon;
}