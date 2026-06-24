#include "CompressorSection.h"

namespace
{
inline static const juce::String dbSymbol = "dB";
inline static const juce::String msSymbol = "ms";
} // namespace

//==============================================================================
//  CompressorSection
//==============================================================================
CompressorSection::CompressorSection(juce::AudioProcessorValueTreeState& apvtsRef, const Params& params,
									 const juce::String& title, juce::Colour accent)
	: apvts(apvtsRef), paramIDs(params), lookAndFeel(std::make_unique<CompressorLookAndFeel>(accent))
{
	sectionLabel.setText(title, juce::dontSendNotification);
	sectionLabel.setFont(juce::FontOptions().withHeight(16.0f));
	sectionLabel.setColour(juce::Label::textColourId, accent.brighter(0.6f));
	sectionLabel.setJustificationType(juce::Justification::centred);
	addAndMakeVisible(sectionLabel);

	// 4 main dials in a 2x2 grid
	attackDial = std::make_unique<DialComponent>(apvts, paramIDs.attack, "Attack", lookAndFeel.get(), msSymbol);
	releaseDial = std::make_unique<DialComponent>(apvts, paramIDs.release, "Release", lookAndFeel.get(), msSymbol);
	thresholdDial =
		std::make_unique<DialComponent>(apvts, paramIDs.threshold, "Threshold", lookAndFeel.get(), dbSymbol);
	ratioDial = std::make_unique<DialComponent>(apvts, paramIDs.ratio, "Ratio", lookAndFeel.get());

	addAndMakeVisible(attackDial.get());
	addAndMakeVisible(releaseDial.get());
	addAndMakeVisible(thresholdDial.get());
	addAndMakeVisible(ratioDial.get());

	// Knee / lookahead toggles
	kneeToggle = std::make_unique<ToggleComponent>(apvts, paramIDs.knee, "Knee", lookAndFeel.get());
	lookaheadToggle = std::make_unique<ToggleComponent>(apvts, paramIDs.lookahead, "Lookahead", lookAndFeel.get());

	addAndMakeVisible(kneeToggle.get());
	addAndMakeVisible(lookaheadToggle.get());

	// Makeup gain dial
	makeupDial = std::make_unique<DialComponent>(apvts, paramIDs.makeupGain, "Makeup", lookAndFeel.get(), dbSymbol);

	addAndMakeVisible(makeupDial.get());
}

void CompressorSection::paint(juce::Graphics& g)
{
	auto bounds = getLocalBounds().toFloat();
	g.setColour(lookAndFeel->getAccentColour().withAlpha(0.08f));
	g.fillRoundedRectangle(bounds.reduced(2.0f), 6.0f);
	g.setColour(lookAndFeel->getAccentColour().withAlpha(0.2f));
	g.drawRoundedRectangle(bounds.reduced(2.0f), 6.0f, 1.0f);
}

void CompressorSection::resized()
{
	auto area = getLocalBounds().reduced(4);
	sectionLabel.setBounds(area.removeFromTop(20));

	// 2x2 grid of dials
	auto dialsArea = area.removeFromTop(static_cast<int>(area.getHeight() * 0.65f));
	auto dialWidth = dialsArea.getWidth() / 2;
	auto dialHeight = dialsArea.getHeight() / 2;

	auto topRow = dialsArea.removeFromTop(dialHeight);
	attackDial->setBounds(topRow.removeFromLeft(dialWidth).reduced(2));
	releaseDial->setBounds(topRow.reduced(2));

	auto bottomRow = dialsArea;
	thresholdDial->setBounds(bottomRow.removeFromLeft(dialWidth).reduced(2));
	ratioDial->setBounds(bottomRow.reduced(2));

	// Toggles — left 65%, makeup gain — right 35% (same width as each main dial)
	auto bottomArea = area;
	auto totalWidth = bottomArea.getWidth();
	auto makeupWidth = totalWidth / 4;
	auto togglesWidth = totalWidth - makeupWidth;
	auto togglesArea = bottomArea.removeFromLeft(togglesWidth);
	auto toggleHeight = static_cast<int>(togglesArea.getHeight() * 0.72f);
	kneeToggle->setBounds(togglesArea.removeFromTop(toggleHeight).reduced(20, 0).translated(6, 0));
	lookaheadToggle->setBounds(kneeToggle->getBounds().translated(0, 30));

	// Makeup gain dial — quarter width, matching the main dials
	makeupDial->setBounds(bottomArea.reduced(2, 6).translated(-28, 0));
}