#pragma once

#include "BaseComponents.h"
#include "LookAndFeel.h"
#include "PluginProcessor.h"

#include <juce_gui_basics/juce_gui_basics.h>

//==============================================================================
/** Saturation section for one band. */
class SaturationSection : public juce::Component
{
  public:
	struct Params
	{
		juce::String drive;
		juce::String evenOdd;
		juce::String heavy;
		juce::String mix;
	};

	SaturationSection(juce::AudioProcessorValueTreeState& apvts, const Params& params, const juce::String& title,
					  juce::Colour accent);

	void resized() override;
	void paint(juce::Graphics& g) override;

  private:
	juce::AudioProcessorValueTreeState& apvts;
	Params paramIDs;
	std::unique_ptr<SectionLookAndFeel> lookAndFeel;
	juce::Label sectionLabel;

	std::unique_ptr<DialComponent> driveDial;
	std::unique_ptr<ToggleComponent> heavyToggle;
	std::unique_ptr<DialComponent> evenOddDial;
	std::unique_ptr<DialComponent> mixDial;
};