#include "LookAndFeel.h"

//==============================================================================
//  SectionLookAndFeel
//==============================================================================
SectionLookAndFeel::SectionLookAndFeel(juce::Colour accentColour) : accent(accentColour) {}

void SectionLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
										  float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider)
{
	juce::ignoreUnused(slider);

	if (! analogueMode)
	{
		// ================================================================
		//  Digital mode: simple modern knob (original implementation)
		// ================================================================
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

		return;
	}

	// ================================================================
	//  Analogue mode: metallic recessed knob with knurled edge
	// ================================================================

	auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(6.0f);
	auto centre = bounds.getCentre();
	auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;

	if (radius < 8.0f)
		return;

	auto angle = rotaryStartAngle + (rotaryEndAngle - rotaryStartAngle) * sliderPos;

	// -- Knurled outer ring (metallic) --
	auto outerRingInnerRadius = radius * 0.88f;
	auto outerRingOuterRadius = radius;

	// Base metallic ring
	g.setColour(juce::Colour(0xFF4A4844));
	g.fillEllipse(centre.getX() - outerRingOuterRadius, centre.getY() - outerRingOuterRadius,
				  outerRingOuterRadius * 2.0f, outerRingOuterRadius * 2.0f);

	// Inner dark ring (shadow recess)
	g.setColour(juce::Colour(0xFF2A2824));
	g.fillEllipse(centre.getX() - outerRingInnerRadius, centre.getY() - outerRingInnerRadius,
				  outerRingInnerRadius * 2.0f, outerRingInnerRadius * 2.0f);

	// -- Knurls (notches around the outer edge) --
	auto numKnurls = juce::jlimit(12, 72, juce::roundToInt(radius * 0.45f));
	auto knurlWidth = juce::MathConstants<float>::twoPi * radius / (float)numKnurls * 0.5f;

	for (int i = 0; i < numKnurls; ++i)
	{
		auto knurlAngle = juce::MathConstants<float>::twoPi * (float)i / (float)numKnurls - juce::MathConstants<float>::halfPi;
		auto innerPt = centre.getPointOnCircumference(outerRingInnerRadius + 1.0f, knurlAngle);
		auto outerPt = centre.getPointOnCircumference(outerRingOuterRadius, knurlAngle);

		// Alternate light/dark for 3D groove effect
		auto knurlColour = (i % 2 == 0) ? juce::Colour(0xFF6A6864) : juce::Colour(0xFF3A3834);
		g.setColour(knurlColour);
		g.drawLine(innerPt.getX(), innerPt.getY(), outerPt.getX(), outerPt.getY(), knurlWidth);
	}

	// -- Bevel highlight on outer ring --
	g.setColour(juce::Colour(0x40FFFFFF));
	g.drawEllipse(centre.getX() - outerRingOuterRadius, centre.getY() - outerRingOuterRadius,
				  outerRingOuterRadius * 2.0f, outerRingOuterRadius * 2.0f, 1.0f);

	g.setColour(juce::Colour(0x40000000));
	g.drawEllipse(centre.getX() - outerRingInnerRadius, centre.getY() - outerRingInnerRadius,
				  outerRingInnerRadius * 2.0f, outerRingInnerRadius * 2.0f, 1.0f);

	// -- Recessed inner face (dark) --
	auto innerRadius = outerRingInnerRadius * 0.78f;
	g.setColour(juce::Colour(0xFF1A1814));
	g.fillEllipse(centre.getX() - innerRadius, centre.getY() - innerRadius,
				  innerRadius * 2.0f, innerRadius * 2.0f);

	// Inner face subtle gradient (top-left highlight)
	g.setColour(juce::Colour(0x18FFFFFF));
	auto highlightRadius = innerRadius * 0.7f;
	g.fillEllipse(centre.getX() - highlightRadius, centre.getY() - innerRadius * 1.3f,
				  highlightRadius * 2.0f, innerRadius * 1.2f);

	// -- Value arc (accent colour) --
	auto arcThickness = radius * 0.07f;
	auto arcRadius = innerRadius * 0.82f;
	juce::Path arc;
	arc.addArc(centre.getX() - arcRadius, centre.getY() - arcRadius,
			   arcRadius * 2.0f, arcRadius * 2.0f,
			   rotaryStartAngle, angle, true);
	g.setColour(accent);
	g.strokePath(arc, juce::PathStrokeType(arcThickness));

	// Arc background (faint track)
	juce::Path arcBg;
	arcBg.addArc(centre.getX() - arcRadius, centre.getY() - arcRadius,
				 arcRadius * 2.0f, arcRadius * 2.0f,
				 rotaryStartAngle, rotaryEndAngle, true);
	g.setColour(juce::Colour(0x30FFFFFF));
	g.strokePath(arcBg, juce::PathStrokeType(arcThickness));

	// -- Pointer / indicator --
	auto pointerLength = innerRadius * 0.7f;
	auto pointerEnd = centre.getPointOnCircumference(pointerLength, angle);

	// Pointer shadow
	g.setColour(juce::Colour(0x40000000));
	auto shadowEnd = centre.getPointOnCircumference(pointerLength + 1.0f, angle + 0.02f);
	g.drawLine(centre.getX() + 1.0f, centre.getY() + 1.0f, shadowEnd.getX(), shadowEnd.getY(), 2.5f);

	// Pointer line
	g.setColour(juce::Colours::white.withAlpha(0.9f));
	g.drawLine(centre.getX(), centre.getY(), pointerEnd.getX(), pointerEnd.getY(), 2.0f);

	// Pointer tip (small triangle/dot)
	g.fillEllipse(pointerEnd.getX() - 2.5f, pointerEnd.getY() - 2.5f, 5.0f, 5.0f);

	// -- Centre cap (metallic screw) --
	auto capRadius = radius * 0.12f;
	g.setColour(juce::Colour(0xFF6A6864));
	g.fillEllipse(centre.getX() - capRadius, centre.getY() - capRadius,
				  capRadius * 2.0f, capRadius * 2.0f);

	// Cap highlight
	g.setColour(juce::Colour(0xAAFFFFFF));
	g.fillEllipse(centre.getX() - capRadius * 0.5f, centre.getY() - capRadius * 0.6f,
				  capRadius * 1.0f, capRadius * 0.8f);

	// Cap screw slot (small line)
	g.setColour(juce::Colour(0x80000000));
	auto screwAngle = angle + juce::MathConstants<float>::halfPi;
	auto screwLen = capRadius * 0.6f;
	auto screwStart = centre.getPointOnCircumference(screwLen * 0.3f, screwAngle);
	auto screwEnd = centre.getPointOnCircumference(screwLen * 0.3f, screwAngle + juce::MathConstants<float>::pi);
	g.drawLine(screwStart.getX(), screwStart.getY(), screwEnd.getX(), screwEnd.getY(), 1.5f);
}

void SectionLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
										  float minSliderPos, float maxSliderPos, juce::Slider::SliderStyle style,
										  juce::Slider& slider)
{
	juce::ignoreUnused(minSliderPos);
	juce::ignoreUnused(maxSliderPos);
	juce::ignoreUnused(style);
	juce::ignoreUnused(slider);

	if (analogueMode)
	{
		// ================================================================
		//  Analogue fader mode: recessed track + tactile fader cap
		// ================================================================
		if (style == juce::Slider::SliderStyle::LinearVertical)
		{
			// --- Fader track (recessed slot) ---
			auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(2.0f);
			auto trackWidth = 8.0f;
			auto trackX = bounds.getCentreX() - trackWidth * 0.5f;
			auto trackBounds = juce::Rectangle<float>(trackX, bounds.getY(), trackWidth, bounds.getHeight());

			// Outer track shadow (recessed look)
			g.setColour(juce::Colour(0x40000000));
			g.fillRoundedRectangle(trackBounds, trackWidth * 0.5f);

			// Dark recessed inner track
			g.setColour(juce::Colour(0xFF2A2824));
			g.fillRoundedRectangle(trackBounds.reduced(1.0f), trackWidth * 0.5f - 1.0f);

			// Slot gradient highlight (top edge light)
			auto slotHighlight = trackBounds.withHeight(trackWidth * 0.5f);
			g.setColour(juce::Colour(0x18FFFFFF));
			g.fillRoundedRectangle(slotHighlight, trackWidth * 0.5f);

			// --- Fader thumb (cap) ---
			auto faderPos = juce::jmap(sliderPos, (float)y, (float)(y + height), bounds.getY(), bounds.getBottom());
			auto capWidth = trackWidth + 32.0f;
			auto capHeight = 12.0f;
			auto capBounds = juce::Rectangle<float>(bounds.getCentreX() - capWidth * 0.5f,
													faderPos - capHeight * 0.5f,
													capWidth, capHeight);

			// Cap body (light grey metallic)
			auto capColour = accent.brighter(0.7f);
			g.setColour(capColour);
			g.fillRoundedRectangle(capBounds, 3.0f);

			// Cap top highlight (3D bevel)
			auto capHighlight = capBounds.withHeight(capHeight * 0.5f);
			g.setColour(juce::Colour(0x55FFFFFF));
			g.fillRoundedRectangle(capHighlight, 3.0f);

			// Cap bottom shadow
			auto capShadow = capBounds.withTop(capBounds.getCentreY());
			g.setColour(juce::Colour(0x33000000));
			g.fillRoundedRectangle(capShadow, 3.0f);

			// Cap outline
			g.setColour(juce::Colour(0x66000000));
			g.drawRoundedRectangle(capBounds, 3.0f, 1.0f);

			// Grip lines on cap (3 horizontal lines)
			g.setColour(juce::Colour(0x44000000));
			auto gripCentreX = capBounds.getCentreX();
			auto gripY = capBounds.getCentreY() - 3.0f;
			for (int i = 0; i < 3; ++i)
			{
				g.drawHorizontalLine(static_cast<int>(gripY + i * 3.0f),
									 gripCentreX - 4.0f, gripCentreX + 4.0f);
			}
		}
		else if (style == juce::Slider::SliderStyle::LinearHorizontal)
		{
			// --- Fader track (recessed slot) ---
			auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(2.0f);
			auto trackHeight = 8.0f;
			auto trackY = bounds.getCentreY() - trackHeight * 0.5f;
			auto trackBounds = juce::Rectangle<float>(bounds.getX(), trackY, bounds.getWidth(), trackHeight);

			// Outer track shadow (recessed look)
			g.setColour(juce::Colour(0x40000000));
			g.fillRoundedRectangle(trackBounds, trackHeight * 0.5f);

			// Dark recessed inner track
			g.setColour(juce::Colour(0xFF2A2824));
			g.fillRoundedRectangle(trackBounds.reduced(1.0f), trackHeight * 0.5f - 1.0f);

			// Slot highlight (top edge)
			auto slotHighlight = trackBounds.withHeight(trackHeight * 0.5f);
			g.setColour(juce::Colour(0x18FFFFFF));
			g.fillRoundedRectangle(slotHighlight, trackHeight * 0.5f);

			// --- Fader thumb (cap) ---
			auto faderPos = juce::jmap(sliderPos, (float)x, (float)(x + width), bounds.getX(), bounds.getRight());
			auto capWidth = 12.0f;
			auto capHeight = trackHeight + 32.0f;
			auto capBounds = juce::Rectangle<float>(faderPos - capWidth * 0.5f,
													bounds.getCentreY() - capHeight * 0.5f,
													capWidth, capHeight);

			// Cap body (light grey metallic)
			auto capColour = accent.brighter(0.7f);
			g.setColour(capColour);
			g.fillRoundedRectangle(capBounds, 3.0f);

			// Cap top highlight (3D bevel)
			auto capHighlight = capBounds.withHeight(capHeight * 0.5f);
			g.setColour(juce::Colour(0x55FFFFFF));
			g.fillRoundedRectangle(capHighlight, 3.0f);

			// Cap bottom shadow
			auto capShadow = capBounds.withTop(capBounds.getCentreY());
			g.setColour(juce::Colour(0x33000000));
			g.fillRoundedRectangle(capShadow, 3.0f);

			// Cap outline
			g.setColour(juce::Colour(0x66000000));
			g.drawRoundedRectangle(capBounds, 3.0f, 1.0f);

			// Grip lines on cap (3 vertical lines)
			g.setColour(juce::Colour(0x44000000));
			auto gripCentreY = capBounds.getCentreY();
			auto gripX = capBounds.getCentreX() - 3.0f;
			for (int i = 0; i < 3; ++i)
			{
				g.drawVerticalLine(static_cast<int>(gripX + i * 3.0f),
								   gripCentreY - 4.0f, gripCentreY + 4.0f);
			}
		}
	}
	else
	{
		// ================================================================
		//  Digital mode: thin modern track / thumb (original)
		// ================================================================
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
}

void SectionLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
										  bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
	juce::ignoreUnused(shouldDrawButtonAsHighlighted);
	juce::ignoreUnused(shouldDrawButtonAsDown);

	auto bounds = button.getLocalBounds().toFloat();

	if (analogueMode)
	{
		// ================================================================
		//  Analogue mode: slide switch (off = left, on = right)
		// ================================================================

		auto switchWidth = 40.0f;
		auto switchHeight = 20.0f;
		auto switchY = bounds.getCentreY() - switchHeight * 0.5f;
		auto switchBounds = juce::Rectangle<float>(0.0f, switchY, switchWidth, switchHeight);

		// --- Track (recessed slot) ---
		// Outer shadow
		g.setColour(juce::Colour(0x40000000));
		g.fillRoundedRectangle(switchBounds, switchHeight * 0.5f);

		// Dark recessed inner
		g.setColour(juce::Colour(0xFF2A2824));
		g.fillRoundedRectangle(switchBounds.reduced(1.5f), switchHeight * 0.5f - 1.0f);

		// Slot top highlight
		auto slotHighlight = switchBounds.withHeight(switchHeight * 0.45f);
		g.setColour(juce::Colour(0x18FFFFFF));
		g.fillRoundedRectangle(slotHighlight, switchHeight * 0.5f);

		// --- Filled portion on the "on" side when active ---
		if (button.getToggleState())
		{
			auto fillBounds = switchBounds.reduced(2.0f);
			auto fillWidth = fillBounds.getWidth() * 0.5f;
			auto fillRegion = juce::Rectangle<float>(fillBounds.getCentreX(), fillBounds.getY(),
													 fillWidth, fillBounds.getHeight());

			g.setColour(accent.withAlpha(0.3f));
			g.fillRoundedRectangle(fillRegion, fillBounds.getHeight() * 0.5f);
		}

		// --- Knob / thumb ---
		auto knobSize = switchHeight - 4.0f;
		auto knobY = switchBounds.getCentreY() - knobSize * 0.5f;
		auto knobX = button.getToggleState()
						 ? switchBounds.getRight() - knobSize - 2.0f
						 : switchBounds.getX() + 2.0f;

		auto knobBounds = juce::Rectangle<float>(knobX, knobY, knobSize, knobSize);
		auto isOn = button.getToggleState();

		// Knob body (coloured with accent)
		auto knobBodyColour = isOn ? accent.brighter(0.3f) : accent.withBrightness(0.35f);
		g.setColour(knobBodyColour);
		g.fillEllipse(knobBounds);

		// Knob top highlight (3D bevel)
		auto knobHighlight = knobBounds.withHeight(knobSize * 0.5f);
		g.setColour(juce::Colour(0xAAFFFFFF));
		g.fillEllipse(knobHighlight.reduced(1.0f));

		// Knob bottom shadow
		auto knobShadow = knobBounds.withTop(knobBounds.getCentreY());
		g.setColour(juce::Colour(0x44000000));
		g.fillEllipse(knobShadow.reduced(1.0f));

		// Knob outline
		g.setColour(juce::Colour(0x99000000));
		g.drawEllipse(knobBounds, 1.0f);

		// Accent dot indicator on the knob when on
		if (isOn)
		{
			g.setColour(accent.brighter(0.6f));
			auto dotBounds = knobBounds.reduced(knobSize * 0.3f);
			g.fillEllipse(dotBounds);
		}

		// --- On/Off labels on the track ---
		g.setColour(juce::Colour(0x80FFFFFF));
		g.setFont(juce::Font(juce::FontOptions().withHeight(8.0f)));
		auto labelY = switchBounds.getCentreY() - 4.0f;

		g.drawFittedText("OFF",
						 juce::Rectangle<float>(switchBounds.getX() + 2.0f, labelY, switchWidth * 0.5f - 2.0f, 8.0f)
							 .toNearestInt(),
						 juce::Justification::centred, 1);

		g.drawFittedText("ON",
						 juce::Rectangle<float>(switchBounds.getCentreX(), labelY, switchWidth * 0.5f - 2.0f, 8.0f)
							 .toNearestInt(),
						 juce::Justification::centred, 1);

		// --- Label text ---
		auto textBounds = bounds.withTrimmedLeft(switchWidth + 8.0f);
		g.setColour(button.findColour(juce::ToggleButton::textColourId));
		g.setFont(juce::Font(juce::FontOptions().withHeight(12.0f)));
		g.drawFittedText(button.getButtonText(), textBounds.toNearestInt(), juce::Justification::centredLeft, 1);

		return;
	}

	// ================================================================
	//  Digital mode: checkbox (original implementation)
	// ================================================================

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
		g.setColour(button.findColour(juce::ToggleButton::textColourId));
		auto tick = juce::Path();
		tick.startNewSubPath(boxBounds.getX() + 3.0f, boxBounds.getCentreY());
		tick.lineTo(boxBounds.getCentreX() - 1.0f, boxBounds.getBottom() - 4.0f);
		tick.lineTo(boxBounds.getRight() - 3.0f, boxBounds.getY() + 4.0f);
		g.strokePath(tick, juce::PathStrokeType(2.0f));
	}

	// Label text
	auto textBounds = bounds.withTrimmedLeft(boxSize + 6.0f);
	g.setColour(button.findColour(juce::ToggleButton::textColourId));
	g.setFont(juce::Font(juce::FontOptions().withHeight(12.0f)));
	g.drawFittedText(button.getButtonText(), textBounds.toNearestInt(), juce::Justification::centredLeft, 1);
}

void SectionLookAndFeel::drawLabel(juce::Graphics& g, juce::Label& label)
{
	if (analogueMode)
	{
		// Analogue mode: no background box, no border, black text
		g.setColour(juce::Colours::black);
		g.setFont(getLabelFont(label));
		g.drawFittedText(label.getText(), label.getLocalBounds().reduced(2, 0), juce::Justification::centred, 1);
	}
	else
	{
		// Digital mode: rounded background
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