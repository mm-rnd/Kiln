#include "BaseComponents.h"

namespace
{
inline static const juce::String dbSymbol = "dB";
inline static const juce::String hzSymbol = "Hz";
inline static const juce::String msSymbol = "ms";
inline static const juce::String pctSymbol = "%";
} // namespace

//==============================================================================
//  DialComponent
//==============================================================================
DialComponent::DialComponent(juce::AudioProcessorValueTreeState& apvts, const juce::String& paramID,
							 const juce::String& labelText, juce::LookAndFeel* lf, const juce::String& suffix)
	: attachment(apvts, paramID, slider)
{
	slider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
	slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 16);
	slider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
	slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::black.withAlpha(0.4f));
	slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
	slider.setLookAndFeel(lf);
	if (suffix.length() > 0)
		slider.setTextValueSuffix(suffix);
	addAndMakeVisible(slider);

	label.setText(labelText, juce::dontSendNotification);
	label.setFont(juce::FontOptions().withHeight(12.0f));
	label.setColour(juce::Label::textColourId, juce::Colours::white);
	label.setJustificationType(juce::Justification::centred);
	addAndMakeVisible(label);
}

void DialComponent::resized()
{
	auto area = getLocalBounds();
	label.setBounds(area.removeFromTop(16));
	slider.setBounds(area);
}

//==============================================================================
//  HorizontalSliderComponent
//==============================================================================
HorizontalSliderComponent::HorizontalSliderComponent(juce::AudioProcessorValueTreeState& apvts,
													 const juce::String& paramID, const juce::String& labelText,
													 juce::LookAndFeel* lf, const juce::String& suffix)
	: attachment(apvts, paramID, slider)
{
	slider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
	slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
	slider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
	slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::black.withAlpha(0.4f));
	slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
	slider.setLookAndFeel(lf);
	if (suffix.length() > 0)
		slider.setTextValueSuffix(suffix);
	addAndMakeVisible(slider);

	label.setText(labelText, juce::dontSendNotification);
	label.setFont(juce::FontOptions().withHeight(14.0f));
	label.setColour(juce::Label::textColourId, juce::Colours::white);
	label.setJustificationType(juce::Justification::centredLeft);
	addAndMakeVisible(label);
}

void HorizontalSliderComponent::resized()
{
	auto area = getLocalBounds();
	label.setBounds(area.removeFromLeft(80));
	slider.setBounds(area);
}

//==============================================================================
//  VerticalSliderComponent
//==============================================================================
VerticalSliderComponent::VerticalSliderComponent(juce::AudioProcessorValueTreeState& apvts, const juce::String& paramID,
												 const juce::String& labelText, juce::LookAndFeel* lf,
												 const juce::String& suffix)
	: attachment(apvts, paramID, slider)
{
	slider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
	slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 16);
	slider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
	slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::black.withAlpha(0.4f));
	slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
	slider.setLookAndFeel(lf);
	if (suffix.length() > 0)
		slider.setTextValueSuffix(suffix);
	addAndMakeVisible(slider);

	if (labelText.length() > 0)
	{
		label.setText(labelText, juce::dontSendNotification);
		label.setFont(juce::FontOptions().withHeight(14.0f));
		label.setColour(juce::Label::textColourId, juce::Colours::white);
		label.setJustificationType(juce::Justification::centred);
		addAndMakeVisible(label);
	}
}

void VerticalSliderComponent::resized()
{
	auto area = getLocalBounds();
	if (label.isVisible())
		area.removeFromBottom(18);
	slider.setBounds(area);
}

//==============================================================================
//  ToggleComponent
//==============================================================================
ToggleComponent::ToggleComponent(juce::AudioProcessorValueTreeState& apvts, const juce::String& paramID,
								 const juce::String& labelText, juce::LookAndFeel* lf)
	: attachment(apvts, paramID, button)
{
	button.setButtonText(labelText);
	button.setLookAndFeel(lf);
	addAndMakeVisible(button);
}

void ToggleComponent::resized()
{
    button.setBounds(getLocalBounds());
}

bool ToggleComponent::hitTest(int x, int y)
{
    auto bounds = getLocalBounds();
    constexpr int toggleHeight = 20;
    auto toggleBounds = bounds.withHeight(toggleHeight).withCentre(bounds.getCentre());
    return toggleBounds.contains(x, y);
}
