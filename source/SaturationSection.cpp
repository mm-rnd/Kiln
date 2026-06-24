#include "SaturationSection.h"

namespace
{
inline static const juce::String pctSymbol = "%";
} // namespace

//==============================================================================
//  SaturationSection
//==============================================================================
SaturationSection::SaturationSection(juce::AudioProcessorValueTreeState& apvtsRef, const Params& params,
									 const juce::String& title, juce::Colour accent)
	: apvts(apvtsRef), paramIDs(params), lookAndFeel(std::make_unique<CompressorLookAndFeel>(accent))
{
	sectionLabel.setText(title, juce::dontSendNotification);
	sectionLabel.setFont(juce::FontOptions().withHeight(16.0f));
	sectionLabel.setColour(juce::Label::textColourId, accent.brighter(0.6f));
	sectionLabel.setJustificationType(juce::Justification::centred);
	addAndMakeVisible(sectionLabel);

	// Large drive knob
	driveDial = std::make_unique<DialComponent>(apvts, paramIDs.drive, "Drive", lookAndFeel.get());
	addAndMakeVisible(driveDial.get());

	// Heavy toggle — small button aligned bottom-right of drive
	heavyToggle = std::make_unique<ToggleComponent>(apvts, paramIDs.heavy, "Heavy", lookAndFeel.get());
	addAndMakeVisible(heavyToggle.get());

	// Even/odd and mix knobs
	evenOddDial = std::make_unique<DialComponent>(apvts, paramIDs.evenOdd, "Even/Odd", lookAndFeel.get(), pctSymbol);
	mixDial = std::make_unique<DialComponent>(apvts, paramIDs.mix, "Mix", lookAndFeel.get(), pctSymbol);

	addAndMakeVisible(evenOddDial.get());
	addAndMakeVisible(mixDial.get());

	// When drive is 0, show "Off" and disable the other controls in this section
	// Must be set up AFTER all controls are created so the initial callback doesn't crash.
	driveDial->getSlider().onValueChange = [this]()
	{
		const auto val = driveDial->getSlider().getValue();
		const bool isOff = (val == 0.0);

		if (isOff)
		{
			driveDial->getSlider().textFromValueFunction = [](double) { return juce::String("Off"); };
		}
		else
		{
			driveDial->getSlider().textFromValueFunction = nullptr;
		}
		driveDial->getSlider().updateText();

		heavyToggle->setEnabled(!isOff);
		evenOddDial->setEnabled(!isOff);
		mixDial->setEnabled(!isOff);
	};
	// Drive the initial state
	driveDial->getSlider().onValueChange();
}

void SaturationSection::paint(juce::Graphics& g)
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

void SaturationSection::resized()
{
	auto area = getLocalBounds().reduced(4);
	sectionLabel.setBounds(area.removeFromTop(20));

	// Split into left (drive + heavy) and right (even/odd + mix)
	auto leftArea = area.removeFromLeft(area.getWidth() / 2);

	// Drive knob — top of left half (70%)
	auto driveArea = leftArea.removeFromTop(static_cast<int>(leftArea.getHeight() * 0.7f));
	driveDial->setBounds(driveArea.reduced(4));

	// Heavy toggle — aligned with drive dial visual left edge
	heavyToggle->setBounds(leftArea.reduced(15, 4));

	// Even/odd and mix — full right half, stacked vertically
	auto rightArea = area.reduced(4);
	auto halfHeight = rightArea.getHeight() / 2;
	evenOddDial->setBounds(rightArea.removeFromTop(halfHeight).reduced(2));
	mixDial->setBounds(rightArea.reduced(2));
}