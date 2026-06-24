#include "LogoSection.h"

#include <BinaryData.h>

//==============================================================================
//  LogoSection
//==============================================================================
LogoSection::LogoSection(const juce::String& url)
{
	// Try to load the logo image from the assets folder
	const juce::Image image = juce::ImageCache::getFromMemory(BinaryData::logo_png, BinaryData::logo_pngSize);
	logoImage = std::make_unique<LinkedImage>(image, url);
	addAndMakeVisible(*logoImage);
}

void LogoSection::paint(juce::Graphics& g)
{
	// No background or border in analogue mode
	if (SectionLookAndFeel::isAnalogueMode())
		return;

	auto bounds = getLocalBounds().toFloat();

	g.setColour(juce::Colour(0xFF1A1A2E));
	g.fillRoundedRectangle(bounds.reduced(2.0f), 6.0f);
}

void LogoSection::resized()
{
	logoImage->setBounds(getLocalBounds().reduced(2));
}