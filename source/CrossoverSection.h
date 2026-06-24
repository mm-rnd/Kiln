#pragma once

#include "BaseComponents.h"
#include "LookAndFeel.h"
#include "PluginProcessor.h"

#include <juce_gui_basics/juce_gui_basics.h>

//==============================================================================
/** Crossover section — two horizontal sliders (low/high crossover frequencies). */
class CrossoverSection : public juce::Component
{
  public:
	CrossoverSection(juce::AudioProcessorValueTreeState& apvts, const juce::String& title);

	void paint(juce::Graphics& g) override;
	void resized() override;

  private:
	juce::AudioProcessorValueTreeState& apvts;
	std::unique_ptr<SectionLookAndFeel> lookAndFeel;
	juce::Label sectionLabel;
	std::unique_ptr<HorizontalSliderComponent> lowXover;
	std::unique_ptr<HorizontalSliderComponent> highXover;
};
