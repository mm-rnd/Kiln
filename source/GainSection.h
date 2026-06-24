#pragma once

#include "BaseComponents.h"
#include "LookAndFeel.h"
#include "PluginProcessor.h"

#include <juce_gui_basics/juce_gui_basics.h>

//==============================================================================
/** Gain section for one band — vertical slider. */
class GainSection : public juce::Component
{
  public:
	GainSection(juce::AudioProcessorValueTreeState& apvts, const juce::String& paramID, const juce::String& title,
				juce::Colour accent);

	void resized() override;
	void paint(juce::Graphics& g) override;

  private:
	juce::AudioProcessorValueTreeState& apvts;
	juce::String paramID;
	std::unique_ptr<SectionLookAndFeel> lookAndFeel;
	juce::Label sectionLabel;
	std::unique_ptr<VerticalSliderComponent> gainSlider;
};