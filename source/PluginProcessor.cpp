#include "PluginProcessor.h"
#include "ParamUtils.h"
#include "PluginEditor.h"

#include <cmath>

//==============================================================================
AudioPluginAudioProcessor::AudioPluginAudioProcessor()
	: AudioProcessor(BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
						 .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
						 .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
						 ),
	  apvts(*this, nullptr, "Parameters", createParameterLayout())
{
	setLatencySamples(crossover.getGroupDelaySamples() + lowCompressor.getLookaheadDelaySamples());
}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor() {}

//==============================================================================
const juce::String AudioPluginAudioProcessor::getName() const
{
	return JucePlugin_Name;
}

bool AudioPluginAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
	return true;
#else
	return false;
#endif
}

bool AudioPluginAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
	return true;
#else
	return false;
#endif
}

bool AudioPluginAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
	return true;
#else
	return false;
#endif
}

double AudioPluginAudioProcessor::getTailLengthSeconds() const
{
	return 0.0;
}

int AudioPluginAudioProcessor::getNumPrograms()
{
	return 1; // NB: some hosts don't cope very well if you tell them there are 0
			  // programs, so this should be at least 1, even if you're not really
			  // implementing programs.
}

int AudioPluginAudioProcessor::getCurrentProgram()
{
	return 0;
}

void AudioPluginAudioProcessor::setCurrentProgram(int index)
{
	juce::ignoreUnused(index);
}

const juce::String AudioPluginAudioProcessor::getProgramName(int index)
{
	juce::ignoreUnused(index);
	return {};
}

void AudioPluginAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
	juce::ignoreUnused(index, newName);
}

//==============================================================================
void AudioPluginAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
	const bool sampleRateChanged = std::abs(crossover.getSampleRate() - sampleRate) > 0.00001;
	crossover.prepare(sampleRate, samplesPerBlock);

	// Prepare compressors — total latency includes crossover delay + any lookahead
	const auto numChannels = getTotalNumOutputChannels();
	lowCompressor.prepare(sampleRate, samplesPerBlock, numChannels);
	midCompressor.prepare(sampleRate, samplesPerBlock, numChannels);
	highCompressor.prepare(sampleRate, samplesPerBlock, numChannels);

	// Prepare saturation stages
	lowSaturation.prepare(sampleRate, samplesPerBlock);
	midSaturation.prepare(sampleRate, samplesPerBlock);
	highSaturation.prepare(sampleRate, samplesPerBlock);

	// Prepare output gain stages
	lowGain.prepare(sampleRate);
	midGain.prepare(sampleRate);
	highGain.prepare(sampleRate);

	if (sampleRateChanged)
	{
		// Total latency: crossover delay + fixed 2 ms delay (always on for all
		// compressor instances to keep bands phase-aligned).
		setLatencySamples(crossover.getGroupDelaySamples() + lowCompressor.getLookaheadDelaySamples());
	}
}

void AudioPluginAudioProcessor::releaseResources()
{
	// When playback stops, you can use this as an opportunity to free up any
	// spare memory, etc.
}

bool AudioPluginAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
	juce::ignoreUnused(layouts);
	return true;
#else
	// This is the place where you check if the layout is supported.
	// In this template code we only support mono or stereo.
	// Some plugin hosts, such as certain GarageBand versions, will only
	// load plugins that support stereo bus layouts.
	if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
		return false;

		// This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
	if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
		return false;
#endif

	return true;
#endif
}

/** Map a ratio choice index to its numeric value. */
static constexpr float ratioChoices[] = {1.0f, 2.0f, 4.0f, 8.0f, 12.0f, 20.0f};
static constexpr int numRatioChoices = 6;

/** Helper: set a Saturation's parameters from APVTS values using band-prefixed params. */
static void applySaturationParams(Saturation& sat, juce::AudioProcessorValueTreeState& apvts, Param drive,
								  Param evenOdd, Param heavy, Param mix)
{
	sat.setDrive(apvts.getRawParameterValue(ParamUtils::toIdentifier(drive))->load());
	sat.setEvenOddBalance(apvts.getRawParameterValue(ParamUtils::toIdentifier(evenOdd))->load());
	const auto heavyParam =
		dynamic_cast<juce::AudioParameterBool*>(apvts.getParameter(ParamUtils::toIdentifier(heavy)));
	sat.setHeavyMode(heavyParam->get());
	sat.setMix(apvts.getRawParameterValue(ParamUtils::toIdentifier(mix))->load());
}

/** Helper: set a Gain's parameters from APVTS values using band-prefixed params. */
static void applyGainParams(Gain& gain, juce::AudioProcessorValueTreeState& apvts, Param outputGain)
{
	gain.setGain(apvts.getRawParameterValue(ParamUtils::toIdentifier(outputGain))->load());
}

/** Helper: set a Compressor's parameters from APVTS values using band-prefixed params. */
static void applyCompressorParams(Compressor& comp, juce::AudioProcessorValueTreeState& apvts, Param attack,
								  Param release, Param threshold, Param ratio, Param knee, Param lookahead,
								  Param makeupGain)
{
	comp.setAttack(apvts.getRawParameterValue(ParamUtils::toIdentifier(attack))->load());
	comp.setRelease(apvts.getRawParameterValue(ParamUtils::toIdentifier(release))->load());
	comp.setThreshold(apvts.getRawParameterValue(ParamUtils::toIdentifier(threshold))->load());
	{
		const auto param =
			dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter(ParamUtils::toIdentifier(ratio)));
		const int clampedIndex = juce::jlimit(0, numRatioChoices - 1, param->getIndex());
		comp.setRatio(ratioChoices[clampedIndex]);
	}
	const auto kneeParam = dynamic_cast<juce::AudioParameterBool*>(apvts.getParameter(ParamUtils::toIdentifier(knee)));
	comp.setKnee(kneeParam->get());
	const auto lookAheadParam =
		dynamic_cast<juce::AudioParameterBool*>(apvts.getParameter(ParamUtils::toIdentifier(lookahead)));
	comp.setLookaheadEnabled(lookAheadParam->get());
	comp.setMakeupGain(apvts.getRawParameterValue(ParamUtils::toIdentifier(makeupGain))->load());
}

void AudioPluginAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	juce::ignoreUnused(midiMessages);

	juce::ScopedNoDenormals noDenormals;

	// --- Set crossover parameters ---
	const auto lowXover = apvts.getRawParameterValue(ParamUtils::toIdentifier(Param::LowXoverDb))->load();
	const auto highXover = apvts.getRawParameterValue(ParamUtils::toIdentifier(Param::HighXoverDb))->load();
	crossover.setLowCrossover(lowXover);
	crossover.setHighCrossover(highXover);

	// --- Set compressor parameters for each band ---
	applyCompressorParams(lowCompressor, apvts, Param::LowAttack, Param::LowRelease, Param::LowThreshold,
						  Param::LowRatio, Param::LowKnee, Param::LowLookahead, Param::LowMakeupGain);
	applyCompressorParams(midCompressor, apvts, Param::MidAttack, Param::MidRelease, Param::MidThreshold,
						  Param::MidRatio, Param::MidKnee, Param::MidLookahead, Param::MidMakeupGain);
	applyCompressorParams(highCompressor, apvts, Param::HighAttack, Param::HighRelease, Param::HighThreshold,
						  Param::HighRatio, Param::HighKnee, Param::HighLookahead, Param::HighMakeupGain);

	// --- Set saturation parameters for each band ---
	applySaturationParams(lowSaturation, apvts, Param::LowSaturationDrive, Param::LowSaturationEvenOdd,
						  Param::LowSaturationHeavy, Param::LowSaturationMix);
	applySaturationParams(midSaturation, apvts, Param::MidSaturationDrive, Param::MidSaturationEvenOdd,
						  Param::MidSaturationHeavy, Param::MidSaturationMix);
	applySaturationParams(highSaturation, apvts, Param::HighSaturationDrive, Param::HighSaturationEvenOdd,
						  Param::HighSaturationHeavy, Param::HighSaturationMix);

	// --- Set output gain parameters for each band ---
	applyGainParams(lowGain, apvts, Param::LowOutputGain);
	applyGainParams(midGain, apvts, Param::MidOutputGain);
	applyGainParams(highGain, apvts, Param::HighOutputGain);

	// --- Split into bands ---
	crossover.split(buffer);

	const int numChannels = buffer.getNumChannels();
	const int numSamples = buffer.getNumSamples();

	// --- Temporary buffers for compressor output ---
	juce::AudioBuffer<float> lowOut(numChannels, numSamples);
	juce::AudioBuffer<float> midOut(numChannels, numSamples);
	juce::AudioBuffer<float> highOut(numChannels, numSamples);

	// Build channel pointer arrays for the compressor's process() method
	std::vector<const float*> chIn(static_cast<size_t>(numChannels));
	std::vector<float*> chOut(static_cast<size_t>(numChannels));

	// --- Compress low band ---
	for (int ch = 0; ch < numChannels; ++ch)
	{
		chIn[static_cast<size_t>(ch)] = crossover.getLowBand(ch);
		chOut[static_cast<size_t>(ch)] = lowOut.getWritePointer(ch);
	}
	lowCompressor.process(chIn.data(), chOut.data(), numSamples);

	// --- Compress mid band ---
	for (int ch = 0; ch < numChannels; ++ch)
	{
		chIn[static_cast<size_t>(ch)] = crossover.getMidBand(ch);
		chOut[static_cast<size_t>(ch)] = midOut.getWritePointer(ch);
	}
	midCompressor.process(chIn.data(), chOut.data(), numSamples);

	// --- Compress high band ---
	for (int ch = 0; ch < numChannels; ++ch)
	{
		chIn[static_cast<size_t>(ch)] = crossover.getHighBand(ch);
		chOut[static_cast<size_t>(ch)] = highOut.getWritePointer(ch);
	}
	highCompressor.process(chIn.data(), chOut.data(), numSamples);

	// --- Apply saturation to each compressed band ---
	lowSaturation.process(lowOut);
	midSaturation.process(midOut);
	highSaturation.process(highOut);

	// --- Apply output gain to each band ---
	lowGain.process(lowOut);
	midGain.process(midOut);
	highGain.process(highOut);

	// --- Sum compressed bands into output buffer ---
	for (int ch = 0; ch < numChannels; ++ch)
	{
		auto* out = buffer.getWritePointer(ch);
		const auto* l = lowOut.getReadPointer(ch);
		const auto* m = midOut.getReadPointer(ch);
		const auto* h = highOut.getReadPointer(ch);

		for (int n = 0; n < numSamples; ++n)
			out[n] = l[n] + m[n] + h[n];
	}
}

//==============================================================================
bool AudioPluginAudioProcessor::hasEditor() const
{
	return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AudioPluginAudioProcessor::createEditor()
{
	return new AudioPluginAudioProcessorEditor(*this);
}

//==============================================================================
void AudioPluginAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
	// You should use this method to store your parameters in the memory block.
	// You could do that either as raw data, or use the XML or ValueTree classes
	// as intermediaries to make it easy to save and load complex data.
	juce::ignoreUnused(destData);
}

void AudioPluginAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
	// You should use this method to restore your parameters from this memory
	// block, whose contents will have been created by the getStateInformation()
	// call.
	juce::ignoreUnused(data, sizeInBytes);
}

juce::AudioProcessorValueTreeState::ParameterLayout AudioPluginAudioProcessor::createParameterLayout()
{
	juce::AudioProcessorValueTreeState::ParameterLayout layout;

	// --- Crossover Parameters ---
	layout.add(std::make_unique<juce::AudioProcessorParameterGroup>(
		"Crossover", "Crossover", ":",
		std::make_unique<juce::AudioParameterFloat>(ParamUtils::toParameterID(Param::LowXoverDb),
													ParamUtils::toName(Param::LowXoverDb),
													juce::NormalisableRange(60.f, 200.0f, 10.f), 120.0f,
													juce::AudioParameterFloatAttributes().withAutomatable(false)),
		std::make_unique<juce::AudioParameterFloat>(ParamUtils::toParameterID(Param::HighXoverDb),
													ParamUtils::toName(Param::HighXoverDb),
													juce::NormalisableRange(2000.0f, 14000.0f, 100.f), 8000.0f,
													juce::AudioParameterFloatAttributes().withAutomatable(false))));

	// --- Helper to add a band's compressor parameters ---
	auto addBandGroup = [&](const juce::String& id, const juce::String& name, Param attack, Param release,
							Param threshold, Param ratio, Param knee, Param lookahead, Param makeupGain)
	{
		const juce::StringArray ratioNames = {"1:1", "2:1", "4:1", "8:1", "12:1", "20:1"};

		layout.add(std::make_unique<juce::AudioProcessorParameterGroup>(
			id, name, ":",
			std::make_unique<juce::AudioParameterFloat>(ParamUtils::toParameterID(attack), ParamUtils::toName(attack),
														juce::NormalisableRange(0.1f, 100.0f, 0.1f), 10.0f,
														juce::AudioParameterFloatAttributes().withAutomatable(true)),
			std::make_unique<juce::AudioParameterFloat>(ParamUtils::toParameterID(release), ParamUtils::toName(release),
														juce::NormalisableRange(1.0f, 1000.0f, 1.0f), 100.0f,
														juce::AudioParameterFloatAttributes().withAutomatable(true)),
			std::make_unique<juce::AudioParameterFloat>(ParamUtils::toParameterID(threshold),
														ParamUtils::toName(threshold),
														juce::NormalisableRange(-60.0f, 0.0f, 0.5f), -24.0f,
														juce::AudioParameterFloatAttributes().withAutomatable(true)),
			std::make_unique<juce::AudioParameterChoice>(ParamUtils::toParameterID(ratio), ParamUtils::toName(ratio),
														 ratioNames, 2), // 2 = index for 4:1
			std::make_unique<juce::AudioParameterBool>(ParamUtils::toParameterID(knee), ParamUtils::toName(knee), true,
													   juce::AudioParameterBoolAttributes().withAutomatable(false)),
			std::make_unique<juce::AudioParameterBool>(ParamUtils::toParameterID(lookahead),
													   ParamUtils::toName(lookahead), false,
													   juce::AudioParameterBoolAttributes().withAutomatable(false)),
			std::make_unique<juce::AudioParameterFloat>(ParamUtils::toParameterID(makeupGain),
														ParamUtils::toName(makeupGain),
														juce::NormalisableRange(0.0f, 24.0f, 0.1f), 0.0f,
														juce::AudioParameterFloatAttributes().withAutomatable(true))));
	};

	addBandGroup("LowComp", "Low Compressor", Param::LowAttack, Param::LowRelease, Param::LowThreshold, Param::LowRatio,
				 Param::LowKnee, Param::LowLookahead, Param::LowMakeupGain);
	addBandGroup("MidComp", "Mid Compressor", Param::MidAttack, Param::MidRelease, Param::MidThreshold, Param::MidRatio,
				 Param::MidKnee, Param::MidLookahead, Param::MidMakeupGain);
	addBandGroup("HighComp", "High Compressor", Param::HighAttack, Param::HighRelease, Param::HighThreshold,
				 Param::HighRatio, Param::HighKnee, Param::HighLookahead, Param::HighMakeupGain);

	// --- Helper to add a band's saturation parameters ---
	auto addSaturationGroup =
		[&](const juce::String& id, const juce::String& name, Param drive, Param evenOdd, Param heavy, Param mix)
	{
		layout.add(std::make_unique<juce::AudioProcessorParameterGroup>(
			id, name, ":",
			std::make_unique<juce::AudioParameterFloat>(ParamUtils::toParameterID(drive), ParamUtils::toName(drive),
														juce::NormalisableRange(0.0f, 100.0f, 1.0f), 0.0f,
														juce::AudioParameterFloatAttributes().withAutomatable(true)),
			std::make_unique<juce::AudioParameterFloat>(ParamUtils::toParameterID(evenOdd), ParamUtils::toName(evenOdd),
														juce::NormalisableRange(-100.0f, 100.0f, 1.0f), 0.0f,
														juce::AudioParameterFloatAttributes().withAutomatable(true)),
			std::make_unique<juce::AudioParameterBool>(ParamUtils::toParameterID(heavy), ParamUtils::toName(heavy),
													   false,
													   juce::AudioParameterBoolAttributes().withAutomatable(false)),
			std::make_unique<juce::AudioParameterFloat>(ParamUtils::toParameterID(mix), ParamUtils::toName(mix),
														juce::NormalisableRange(-100.0f, 100.0f, 1.0f), 0.0f,
														juce::AudioParameterFloatAttributes().withAutomatable(true))));
	};

	addSaturationGroup("LowSat", "Low Saturation", Param::LowSaturationDrive, Param::LowSaturationEvenOdd,
					   Param::LowSaturationHeavy, Param::LowSaturationMix);
	addSaturationGroup("MidSat", "Mid Saturation", Param::MidSaturationDrive, Param::MidSaturationEvenOdd,
					   Param::MidSaturationHeavy, Param::MidSaturationMix);
	addSaturationGroup("HighSat", "High Saturation", Param::HighSaturationDrive, Param::HighSaturationEvenOdd,
					   Param::HighSaturationHeavy, Param::HighSaturationMix);

	// --- Helper to add a band's output gain parameters ---
	auto addOutputGainGroup = [&](const juce::String& id, const juce::String& name, Param outputGain)
	{
		layout.add(std::make_unique<juce::AudioProcessorParameterGroup>(
			id, name, ":",
			std::make_unique<juce::AudioParameterFloat>(ParamUtils::toParameterID(outputGain),
														ParamUtils::toName(outputGain),
														juce::NormalisableRange(-24.0f, 24.0f, 0.1f), 0.0f,
														juce::AudioParameterFloatAttributes().withAutomatable(true))));
	};

	addOutputGainGroup("LowGain", "Low Output", Param::LowOutputGain);
	addOutputGainGroup("MidGain", "Mid Output", Param::MidOutputGain);
	addOutputGainGroup("HighGain", "High Output", Param::HighOutputGain);

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
	return new AudioPluginAudioProcessor();
}