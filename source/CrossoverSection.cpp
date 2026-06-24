#include "CrossoverSection.h"

namespace
{
inline static const juce::String hzSymbol = "Hz";
} // namespace

//==============================================================================
//  CrossoverSection
//==============================================================================
CrossoverSection::CrossoverSection(juce::AudioProcessorValueTreeState& apvtsRef, const juce::String& title)
	: apvts(apvtsRef), lookAndFeel(std::make_unique<SliderLookAndFeel>(juce::Colour(0xFF4488CC)))
{
	sectionLabel.setText(title, juce::dontSendNotification);
	sectionLabel.setFont(juce::FontOptions().withHeight(16.0f));
	sectionLabel.setColour(juce::Label::textColourId, lookAndFeel->getAccentColour().brighter(0.6f));
	sectionLabel.setJustificationType(juce::Justification::centred);
	addAndMakeVisible(sectionLabel);

	lowXover = std::make_unique<HorizontalSliderComponent>(apvts, ParamUtils::toIdentifier(Param::LowXoverDb),
														   "Low Xover", lookAndFeel.get(), hzSymbol);

	highXover = std::make_unique<HorizontalSliderComponent>(apvts, ParamUtils::toIdentifier(Param::HighXoverDb),
															"High Xover", lookAndFeel.get(), hzSymbol);

	addAndMakeVisible(lowXover.get());
	addAndMakeVisible(highXover.get());
}

void CrossoverSection::paint(juce::Graphics& g)
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

void CrossoverSection::resized()
{
	auto area = getLocalBounds().reduced(4);
	sectionLabel.setBounds(area.removeFromTop(20));

	auto slidersArea = area;
	auto halfWidth = slidersArea.getWidth() / 2;
	lowXover->setBounds(slidersArea.removeFromLeft(halfWidth).reduced(4));
	highXover->setBounds(slidersArea.reduced(4));
}
