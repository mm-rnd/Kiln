#include "GainSection.h"

namespace
{
inline static const juce::String dbSymbol = "dB";
} // namespace

//==============================================================================
//  GainSection
//==============================================================================
GainSection::GainSection(juce::AudioProcessorValueTreeState& apvtsRef, const juce::String& paramIDStr,
						 const juce::String& title, juce::Colour accent)
	: apvts(apvtsRef), paramID(paramIDStr), lookAndFeel(std::make_unique<SliderLookAndFeel>(accent))
{
	sectionLabel.setText(title, juce::dontSendNotification);
	sectionLabel.setFont(juce::FontOptions().withHeight(16.0f));
	sectionLabel.setColour(juce::Label::textColourId, accent.brighter(0.6f));
	sectionLabel.setJustificationType(juce::Justification::centred);
	addAndMakeVisible(sectionLabel);

	gainSlider = std::make_unique<VerticalSliderComponent>(apvts, paramID, "", lookAndFeel.get(), dbSymbol);
	addAndMakeVisible(gainSlider.get());
}

void GainSection::paint(juce::Graphics& g)
{
	auto bounds = getLocalBounds().toFloat();

	if (SectionLookAndFeel::isAnalogueMode())
	{
		g.setColour(SectionLookAndFeel::analogueBorderColour);
		g.drawRoundedRectangle(bounds.reduced(2.0f), 6.0f, 1.0f);
	}
	else
	{
		g.setColour(lookAndFeel->getAccentColour().withAlpha(0.08f));
		g.fillRoundedRectangle(bounds.reduced(2.0f), 6.0f);
		g.setColour(lookAndFeel->getAccentColour().withAlpha(0.2f));
		g.drawRoundedRectangle(bounds.reduced(2.0f), 6.0f, 1.0f);
	}
}

void GainSection::resized()
{
	auto area = getLocalBounds().reduced(4);
	sectionLabel.setBounds(area.removeFromTop(20));
	area.removeFromBottom(6);
	gainSlider->setBounds(area);
}