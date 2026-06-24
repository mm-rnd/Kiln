#pragma once

#include "BaseComponents.h"
#include "LookAndFeel.h"
#include "PluginProcessor.h"

#include <juce_gui_basics/juce_gui_basics.h>

//==============================================================================
/** Limiter section — vertical slider for ceiling. */
class LimiterSection : public juce::Component
{
  public:
	LimiterSection(juce::AudioProcessorValueTreeState& apvts);

	void resized() override;
	void paint(juce::Graphics& g) override;

  private:
	std::unique_ptr<SectionLookAndFeel> lookAndFeel;
	juce::Label sectionLabel;
	std::unique_ptr<HorizontalSliderComponent> ceilingSlider;
};