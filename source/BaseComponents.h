#pragma once

#include "LookAndFeel.h"
#include "PluginProcessor.h"

#include <juce_gui_basics/juce_gui_basics.h>

//==============================================================================
/** Textbox colour scheme for digital (dark) mode. */
inline static const auto textBoxTextColourDigital = juce::Colour(0xFFFFFFFF);
inline static const auto textBoxBackgroundColourDigital = juce::Colour(0x66000000);
inline static const auto textBoxOutlineColourDigital = juce::Colour(0x00000000);

/** Textbox colour scheme for analogue (light/matt) mode. */
inline static const auto textBoxTextColourAnalogue = juce::Colour(0xFF000000);
inline static const auto textBoxBackgroundColourAnalogue = juce::Colour(0x00000000);
inline static const auto textBoxOutlineColourAnalogue = juce::Colour(0x00000000);

//==============================================================================
/** A labelled rotary slider (dial). */
class DialComponent : public juce::Component
{
  public:
	DialComponent(juce::AudioProcessorValueTreeState& apvts, const juce::String& paramID, const juce::String& labelText,
				  juce::LookAndFeel* lf, const juce::String& suffix = "");

	void resized() override;

	juce::Slider& getSlider() noexcept { return slider; }

  private:
	juce::Slider slider;
	juce::Label label;
	juce::AudioProcessorValueTreeState::SliderAttachment attachment;
};

//==============================================================================
/** A horizontal slider with a label. */
class HorizontalSliderComponent : public juce::Component
{
  public:
	HorizontalSliderComponent(juce::AudioProcessorValueTreeState& apvts, const juce::String& paramID,
							  const juce::String& labelText, juce::LookAndFeel* lf, const juce::String& suffix = "");

	void resized() override;

	juce::Slider& getSlider() noexcept { return slider; }

  private:
	juce::Slider slider;
	juce::Label label;
	juce::AudioProcessorValueTreeState::SliderAttachment attachment;
};

//==============================================================================
/** A vertical slider with a label. */
class VerticalSliderComponent : public juce::Component
{
  public:
	VerticalSliderComponent(juce::AudioProcessorValueTreeState& apvts, const juce::String& paramID,
							const juce::String& labelText, juce::LookAndFeel* lf, const juce::String& suffix = "");

	void resized() override;

	juce::Slider& getSlider() noexcept { return slider; }

  private:
	juce::Slider slider;
	juce::Label label;
	juce::AudioProcessorValueTreeState::SliderAttachment attachment;
};

//==============================================================================
/** A toggle button (for bool parameters). */
class ToggleComponent : public juce::Component
{
  public:
	ToggleComponent(juce::AudioProcessorValueTreeState& apvts, const juce::String& paramID,
					const juce::String& labelText, juce::LookAndFeel* lf);

	void resized() override;

	bool hitTest(int x, int y) override;

	void setTextColour(juce::Colour colour);

  private:
	juce::ToggleButton button;
	juce::AudioProcessorValueTreeState::ButtonAttachment attachment;
};