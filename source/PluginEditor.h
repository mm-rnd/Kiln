#pragma once

#include "PluginProcessor.h"

#include "AudioAnalyser.h"
#include "LinkedImage.h"
#include "LookAndFeel.h"
#include "BaseComponents.h"
#include "CrossoverSection.h"
#include "CompressorSection.h"
#include "SaturationSection.h"
#include "GainSection.h"
#include "LimiterSection.h"
#include "LogoSection.h"

#include <juce_gui_basics/juce_gui_basics.h>

//==============================================================================
/** Top-level editor component. */
class AudioPluginAudioProcessorEditor final : public juce::AudioProcessorEditor
{
  public:
	explicit AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor&);
	~AudioPluginAudioProcessorEditor() override;

	void paint(juce::Graphics&) override;
	void resized() override;

  private:
	AudioPluginAudioProcessor& processorRef;
	juce::AudioProcessorValueTreeState& apvts;

	// Audio analyser display
	std::unique_ptr<AudioAnalyser> analyser;

	// Crossover section
	CrossoverSection crossoverSection;

	// Three band sections — each containing compressor, saturation, gain
	std::unique_ptr<juce::FlexBox> bandsFlexBox;
	std::unique_ptr<LimiterSection> limiterSection;
	std::unique_ptr<LogoSection> logoSectionLeft;
	std::unique_ptr<LogoSection> logoSectionRight;

	// Vectors for the band columns and their child sections
	std::vector<std::unique_ptr<juce::Component>> bandColumns;
	std::vector<std::unique_ptr<juce::FlexBox>> bandColumnFlexBoxes;
	std::vector<std::unique_ptr<juce::Component>> bandColumnLeftContainers;
	std::vector<std::unique_ptr<juce::FlexBox>> bandColumnLeftFlexes;
	std::vector<std::unique_ptr<CompressorSection>> compSections;
	std::vector<std::unique_ptr<SaturationSection>> satSections;
	std::vector<std::unique_ptr<GainSection>> gainSections;

	static constexpr int analyserHeight = 120;
	static constexpr int crossoverHeight = 80;
	static constexpr int limiterHeight = 200;
	static constexpr int bandWidth = 320;

	CompressorSection::Params makeCompParams(const juce::String& prefix);
	SaturationSection::Params makeSatParams(const juce::String& prefix);

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessorEditor)
};