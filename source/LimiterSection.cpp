#include "LimiterSection.h"

namespace
{
inline static const juce::String dbSymbol = "dB";
} // namespace

//==============================================================================
//  LimiterSection
//==============================================================================
LimiterSection::LimiterSection(juce::AudioProcessorValueTreeState& apvts)
	: lookAndFeel(std::make_unique<SliderLookAndFeel>(juce::Colour(0xFF8844CC)))
{
	sectionLabel.setText("Limiter", juce::dontSendNotification);
	sectionLabel.setFont(juce::FontOptions().withHeight(16.0f));
	sectionLabel.setColour(juce::Label::textColourId, juce::Colour(0xFF8844CC).brighter(0.6f));
	sectionLabel.setJustificationType(juce::Justification::centred);
	addAndMakeVisible(sectionLabel);

	ceilingSlider = std::make_unique<HorizontalSliderComponent>(apvts, ParamUtils::toIdentifier(Param::LimiterCeiling),
																"Ceiling", lookAndFeel.get(), dbSymbol);

	addAndMakeVisible(ceilingSlider.get());
}

void LimiterSection::paint(juce::Graphics& g)
{
	auto bounds = getLocalBounds().toFloat();
	g.setColour(lookAndFeel->getAccentColour().withAlpha(0.08f));
	g.fillRoundedRectangle(bounds.reduced(2.0f), 6.0f);
	g.setColour(lookAndFeel->getAccentColour().withAlpha(0.2f));
	g.drawRoundedRectangle(bounds.reduced(2.0f), 6.0f, 1.0f);
}

void LimiterSection::resized()
{
	auto area = getLocalBounds().reduced(18, 4);
	sectionLabel.setBounds(area.removeFromTop(20));
	ceilingSlider->setBounds(area);
}