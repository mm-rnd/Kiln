#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

//==============================================================================
/** Base look-and-feel for a section, providing a distinct accent colour. */
class SectionLookAndFeel : public juce::LookAndFeel_V4
{
  public:
	explicit SectionLookAndFeel(juce::Colour accentColour);
	juce::Colour getAccentColour() const noexcept { return accent; }

	void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
						  float rotaryStartAngle, float rotaryEndAngle, juce::Slider&) override;

	void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos, float minSliderPos,
						  float maxSliderPos, juce::Slider::SliderStyle style, juce::Slider& slider) override;

	void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button, bool shouldDrawButtonAsHighlighted,
						  bool shouldDrawButtonAsDown) override;

	void drawLabel(juce::Graphics& g, juce::Label& label) override;

  private:
	juce::Colour accent;
};

//==============================================================================
/** Compressor-specific look-and-feel with custom label font for slider labels. */
class CompressorLookAndFeel : public SectionLookAndFeel
{
  public:
	explicit CompressorLookAndFeel(juce::Colour accentColour) : SectionLookAndFeel(accentColour) {}

	juce::Font getLabelFont(juce::Label& label) override;

  private:
	juce::Colour accent;
};

//==============================================================================
/** Slider look and feel class. */
class SliderLookAndFeel : public SectionLookAndFeel
{
  public:
	explicit SliderLookAndFeel(juce::Colour accentColour) : SectionLookAndFeel(accentColour) {}

	juce::Font getLabelFont(juce::Label& label) override;

  private:
	juce::Colour accent;
};
