#include "LookAndFeel.h"

//==============================================================================
//  SectionLookAndFeel
//==============================================================================
SectionLookAndFeel::SectionLookAndFeel(juce::Colour accentColour) : accent(accentColour) {}

void SectionLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
										  float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider)
{
	juce::ignoreUnused(slider);

	auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(4.0f);
	auto centre = bounds.getCentre();
	auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;

	// Background circle
	g.setColour(accent.withBrightness(0.2f));
	g.fillEllipse(centre.getX() - radius, centre.getY() - radius, radius * 2.0f, radius * 2.0f);

	// Outline
	g.setColour(accent.withAlpha(0.6f));
	g.drawEllipse(centre.getX() - radius, centre.getY() - radius, radius * 2.0f, radius * 2.0f, 1.5f);

	// Arc
	auto angle = rotaryStartAngle + (rotaryEndAngle - rotaryStartAngle) * sliderPos;
	juce::Path arc;
	arc.addArc(centre.getX() - radius, centre.getY() - radius, radius * 2.0f, radius * 2.0f, rotaryStartAngle, angle,
			   true);
	g.setColour(accent);
	g.strokePath(arc, juce::PathStrokeType(2.5f));

	// Pointer line
	auto pointerLength = radius * 0.6f;
	auto pointerEnd = centre.getPointOnCircumference(pointerLength, angle);
	g.drawLine(centre.getX(), centre.getY(), pointerEnd.getX(), pointerEnd.getY(), 2.0f);

	// Centre dot
	g.setColour(accent.brighter(0.5f));
	g.fillEllipse(centre.getX() - 2.5f, centre.getY() - 2.5f, 5.0f, 5.0f);
}

void SectionLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
										  float minSliderPos, float maxSliderPos, juce::Slider::SliderStyle style,
										  juce::Slider& slider)
{
	juce::ignoreUnused(minSliderPos);
	juce::ignoreUnused(maxSliderPos);
	juce::ignoreUnused(style);
	juce::ignoreUnused(slider);

	if (style == juce::Slider::SliderStyle::LinearVertical)
	{
		auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(2.0f);
		auto trackWidth = 4.0f;
		auto trackX = bounds.getCentreX() - trackWidth * 0.5f;

		// Track background
		g.setColour(accent.withBrightness(0.15f));
		g.fillRoundedRectangle(trackX, bounds.getY(), trackWidth, bounds.getHeight(), 2.0f);

		// Filled track
		auto fillEnd = juce::jmap(sliderPos, (float)y, (float)(y + height), bounds.getY(), bounds.getBottom());
		g.setColour(accent);
		g.fillRoundedRectangle(trackX, fillEnd, trackWidth, bounds.getBottom() - fillEnd, 2.0f);

		// Thumb
		auto thumbSize = 12.0f;
		g.setColour(accent.brighter(0.3f));
		g.fillRoundedRectangle(trackX - (thumbSize - trackWidth) * 0.5f, fillEnd - thumbSize * 0.5f, thumbSize,
							   thumbSize, 3.0f);
	}
	else if (style == juce::Slider::SliderStyle::LinearHorizontal)
	{
		auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(2.0f);
		auto trackHeight = 4.0f;
		auto trackY = bounds.getCentreY() - trackHeight * 0.5f;

		// Track background
		g.setColour(accent.withBrightness(0.15f));
		g.fillRoundedRectangle(bounds.getX(), trackY, bounds.getWidth(), trackHeight, 2.0f);

		// Filled track
		auto fillEnd = juce::jmap(sliderPos, (float)x, (float)(x + width), bounds.getX(), bounds.getRight());
		g.setColour(accent);
		g.fillRoundedRectangle(bounds.getX(), trackY, fillEnd - bounds.getX(), trackHeight, 2.0f);

		// Thumb
		auto thumbSize = 12.0f;
		g.setColour(accent.brighter(0.3f));
		g.fillRoundedRectangle(fillEnd - thumbSize * 0.5f, trackY - (thumbSize - trackHeight) * 0.5f, thumbSize,
							   thumbSize, 3.0f);
	}
}

void SectionLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
										  bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
	juce::ignoreUnused(shouldDrawButtonAsHighlighted);
	juce::ignoreUnused(shouldDrawButtonAsDown);

	auto bounds = button.getLocalBounds().toFloat();
	auto boxSize = 16.0f;
	auto boxBounds = juce::Rectangle<float>(0.0f, bounds.getCentreY() - boxSize * 0.5f, boxSize, boxSize);

	// Box
	auto boxColour = accent;
	if (button.getToggleState())
		boxColour = accent.brighter(0.4f);
	else
		boxColour = accent.withBrightness(0.2f);

	g.setColour(boxColour);
	g.fillRoundedRectangle(boxBounds, 3.0f);
	g.setColour(accent.withAlpha(0.6f));
	g.drawRoundedRectangle(boxBounds, 3.0f, 1.5f);

	if (button.getToggleState())
	{
		// Tick
		g.setColour(juce::Colours::white);
		auto tick = juce::Path();
		tick.startNewSubPath(boxBounds.getX() + 3.0f, boxBounds.getCentreY());
		tick.lineTo(boxBounds.getCentreX() - 1.0f, boxBounds.getBottom() - 4.0f);
		tick.lineTo(boxBounds.getRight() - 3.0f, boxBounds.getY() + 4.0f);
		g.strokePath(tick, juce::PathStrokeType(2.0f));
	}

	// Label text
	auto textBounds = bounds.withTrimmedLeft(boxSize + 6.0f);
	g.setColour(juce::Colours::white);
	g.setFont(12.0f);
	g.drawFittedText(button.getButtonText(), textBounds.toNearestInt(), juce::Justification::centredLeft, 1);
}

void SectionLookAndFeel::drawLabel(juce::Graphics& g, juce::Label& label)
{
	// Rounded background
	g.setColour(juce::Colour(0xFF2A2A3A));
	g.fillRoundedRectangle(label.getLocalBounds().toFloat(), 3.0f);

	// Optional border
	g.setColour(juce::Colour(0x44FFFFFF));
	g.drawRoundedRectangle(label.getLocalBounds().toFloat().reduced(0.5f), 3.0f, 0.5f);

	// Text
	g.setColour(juce::Colours::white.withAlpha(0.85f));
	g.setFont(getLabelFont(label));
	g.drawFittedText(label.getText(), label.getLocalBounds().reduced(2, 0), juce::Justification::centred, 1);
}

//==============================================================================
//  CompressorLookAndFeel
//==============================================================================
juce::Font CompressorLookAndFeel::getLabelFont(juce::Label& label)
{
	if (auto* parent = label.getParentComponent())
		if (dynamic_cast<juce::Slider*>(parent) != nullptr)
			return juce::Font(juce::FontOptions().withHeight(11.0f));

	return LookAndFeel_V4::getLabelFont(label);
}

//==============================================================================
//  SliderLookAndFeel
//==============================================================================
juce::Font SliderLookAndFeel::getLabelFont(juce::Label& label)
{
	if (auto* parent = label.getParentComponent())
		if (dynamic_cast<juce::Slider*>(parent) != nullptr)
			return juce::Font(juce::FontOptions().withHeight(14.0f));

	return LookAndFeel_V4::getLabelFont(label);
}