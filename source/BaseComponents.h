#pragma once

#include "PluginProcessor.h"
#include "LookAndFeel.h"

#include <juce_gui_basics/juce_gui_basics.h>

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

  private:
	juce::ToggleButton button;
	juce::AudioProcessorValueTreeState::ButtonAttachment attachment;
};