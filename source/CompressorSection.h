#pragma once

#include "BaseComponents.h"
#include "LookAndFeel.h"
#include "PluginProcessor.h"

#include <juce_gui_basics/juce_gui_basics.h>

//==============================================================================
/** Compressor section for one band: 4 knobs (2x2), knee/lookahead toggles, makeup dial. */
class CompressorSection : public juce::Component
{
  public:
	struct Params
	{
		juce::String attack;
		juce::String release;
		juce::String threshold;
		juce::String ratio;
		juce::String knee;
		juce::String lookahead;
		juce::String makeupGain;
	};

	CompressorSection(juce::AudioProcessorValueTreeState& apvts, const Params& params, const juce::String& title,
					  juce::Colour accent);

	void resized() override;
	void paint(juce::Graphics& g) override;

  private:
	juce::AudioProcessorValueTreeState& apvts;
	Params paramIDs;
	std::unique_ptr<CompressorLookAndFeel> lookAndFeel;
	juce::Label sectionLabel;

	// 4 main dials
	std::unique_ptr<DialComponent> attackDial;
	std::unique_ptr<DialComponent> releaseDial;
	std::unique_ptr<DialComponent> thresholdDial;
	std::unique_ptr<DialComponent> ratioDial;

	// Knee / lookahead toggles
	std::unique_ptr<ToggleComponent> kneeToggle;
	std::unique_ptr<ToggleComponent> lookaheadToggle;

	// Makeup gain dial
	std::unique_ptr<DialComponent> makeupDial;
};