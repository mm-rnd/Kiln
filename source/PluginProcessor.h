#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "AudioAnalyser.h"
#include "Compressor.h"
#include "Gain.h"
#include "Limiter.h"
#include "LinearPhaseCrossover.h"
#include "ProgramManager.h"
#include "Saturation.h"

//==============================================================================
class AudioPluginAudioProcessor final : public juce::AudioProcessor
{
  public:
	//==============================================================================
	AudioPluginAudioProcessor();
	~AudioPluginAudioProcessor() override;

	//==============================================================================
	void prepareToPlay(double sampleRate, int samplesPerBlock) override;
	void releaseResources() override;

	bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

	void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
	using AudioProcessor::processBlock;

	//==============================================================================
	juce::AudioProcessorEditor* createEditor() override;
	bool hasEditor() const override;

	//==============================================================================
	const juce::String getName() const override;

	bool acceptsMidi() const override;
	bool producesMidi() const override;
	bool isMidiEffect() const override;
	double getTailLengthSeconds() const override;

	//==============================================================================
	int getNumPrograms() override;
	int getCurrentProgram() override;
	void setCurrentProgram(int index) override;
	const juce::String getProgramName(int index) override;
	void changeProgramName(int index, const juce::String& newName) override;

	//==============================================================================
	void getStateInformation(juce::MemoryBlock& destData) override;
	void setStateInformation(const void* data, int sizeInBytes) override;

	/** Access the program manager (for UI / host integration). */
	[[nodiscard]] ProgramManager& getProgramManager() noexcept { return programManager; }
	[[nodiscard]] const ProgramManager& getProgramManager() const noexcept { return programManager; }

	/** Access the analyser data (for UI). */
	[[nodiscard]] AnalyserData& getAnalyserData() noexcept { return analyserData; }
	[[nodiscard]] const AnalyserData& getAnalyserData() const noexcept { return analyserData; }

	juce::AudioProcessorValueTreeState apvts;

  private:
	juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

	juce::UndoManager undoManager;

	ProgramManager programManager;

	/** Input crossover. */
	LinearPhaseCrossover crossover;

	/** One compressor per crossover band. */
	Compressor lowCompressor;
	Compressor midCompressor;
	Compressor highCompressor;

	/** One saturation stage per crossover band. */
	Saturation lowSaturation;
	Saturation midSaturation;
	Saturation highSaturation;

	/** One output gain stage per crossover band. */
	Gain lowGain;
	Gain midGain;
	Gain highGain;

	/** Output limiter on summed audio. */
	Limiter limiter;

	/** Audio analyser data (shared with UI). */
	AnalyserData analyserData;

	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessor)
};
