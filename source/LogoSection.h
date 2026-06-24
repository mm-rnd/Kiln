#pragma once

#include "LinkedImage.h"
#include "LookAndFeel.h"
#include "PluginProcessor.h"

#include <juce_gui_basics/juce_gui_basics.h>

//==============================================================================
/** Logo section — square component to hold a logo image. */
class LogoSection : public juce::Component
{
  public:
	LogoSection(const juce::String& url);

	void paint(juce::Graphics& g) override;
	void resized() override;

  private:
	std::unique_ptr<LinkedImage> logoImage;
};