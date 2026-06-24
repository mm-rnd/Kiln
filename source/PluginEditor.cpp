#include "PluginEditor.h"
#include "BaseComponents.h"
#include "CompressorSection.h"
#include "CrossoverSection.h"
#include "GainSection.h"
#include "LimiterSection.h"
#include "LogoSection.h"
#include "LookAndFeel.h"
#include "ParamUtils.h"
#include "PluginProcessor.h"
#include "SaturationSection.h"

#include <juce_gui_basics/juce_gui_basics.h>

#include <BinaryData.h>
#include <cctype>

namespace
{
inline static const juce::String dbSymbol = "dB";
inline static const juce::String hzSymbol = "Hz";
inline static const juce::String msSymbol = "ms";
inline static const juce::String pctSymbol = "%";
} // namespace

//==============================================================================
//  AudioPluginAudioProcessorEditor
//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor& p)
	: AudioProcessorEditor(&p), processorRef(p), apvts(p.apvts), crossoverSection(apvts, "Crossover")
{
	setWantsKeyboardFocus(true);

	// Audio analyser display
	AudioAnalyser::BandColours bandColours{
		juce::Colour(0xFF44CC88), // Low — green
		juce::Colour(0xFFCCAA44), // Mid — amber
		juce::Colour(0xFFCC6644)  // High — orange
	};
	analyser = std::make_unique<AudioAnalyser>(processorRef.getAnalyserData(), bandColours);
	addAndMakeVisible(analyser.get());
	analyser->startAnalyser();

	// Crossover section
	addAndMakeVisible(crossoverSection);

	// Build the three band sections using a vector of parameter groups
	struct BandDef
	{
		juce::String prefix;
		juce::Colour colour;
	};

	std::vector<BandDef> bands = {
		{"Low", juce::Colour(0xFF44CC88)}, // Green
		{"Mid", juce::Colour(0xFFCCAA44)}, // Amber
		{"High", juce::Colour(0xFFCC6644)} // Orange
	};

	// We'll use a FlexBox to hold the three band columns
	bandsFlexBox = std::make_unique<juce::FlexBox>();
	bandsFlexBox->flexDirection = juce::FlexBox::Direction::row;
	bandsFlexBox->justifyContent = juce::FlexBox::JustifyContent::flexStart;
	bandsFlexBox->alignItems = juce::FlexBox::AlignItems::stretch;

	for (const auto& band : bands)
	{
		// Create a column for this band: compressor + saturation + gain
		auto bandColumn = std::make_unique<juce::Component>();

		auto compParams = makeCompParams(band.prefix);
		auto compSection = std::make_unique<CompressorSection>(apvts, compParams, band.prefix + " Comp", band.colour);

		auto satParams = makeSatParams(band.prefix);
		auto satSection = std::make_unique<SaturationSection>(apvts, satParams, band.prefix + " Sat",
															  band.colour.withHue(band.colour.getHue() + 0.05f));

		auto gainParamID = (band.prefix == "Low")	? Param::LowOutputGain
						   : (band.prefix == "Mid") ? Param::MidOutputGain
													: Param::HighOutputGain;
		auto gainSection =
			std::make_unique<GainSection>(apvts, ParamUtils::toIdentifier(gainParamID), band.prefix + " Gain",
										  band.colour.withHue(band.colour.getHue() - 0.05f));

		// Use FlexBox within each band column: left side (comp + sat), right side (gain + full height)
		auto columnFlex = std::make_unique<juce::FlexBox>();
		columnFlex->flexDirection = juce::FlexBox::Direction::row;
		columnFlex->justifyContent = juce::FlexBox::JustifyContent::flexStart;
		columnFlex->alignItems = juce::FlexBox::AlignItems::stretch;

		// Inner vertical flex for comp + sat on the left
		auto leftFlex = std::make_unique<juce::FlexBox>();
		leftFlex->flexDirection = juce::FlexBox::Direction::column;
		leftFlex->justifyContent = juce::FlexBox::JustifyContent::flexStart;
		leftFlex->alignItems = juce::FlexBox::AlignItems::stretch;
		leftFlex->items.add(juce::FlexItem(*compSection).withFlex(1.0f).withMinHeight(280));
		leftFlex->items.add(juce::FlexItem(*satSection).withFlex(1.0f).withMinHeight(200));

		// Wrap the left side in a container component so FlexItem can reference it
		auto leftContainer = std::make_unique<juce::Component>();
		leftContainer->addAndMakeVisible(compSection.get());
		leftContainer->addAndMakeVisible(satSection.get());

		columnFlex->items.add(juce::FlexItem(*leftContainer).withFlex(1.0f));
		columnFlex->items.add(juce::FlexItem(*gainSection).withFlex(0.225f).withMinWidth(45));

		bandColumn->addAndMakeVisible(leftContainer.get());
		bandColumn->addAndMakeVisible(gainSection.get());

		bandColumnLeftContainers.push_back(std::move(leftContainer));
		bandColumnLeftFlexes.push_back(std::move(leftFlex));

		// Store the column and its flexbox
		bandColumns.push_back(std::move(bandColumn));
		bandColumnFlexBoxes.push_back(std::move(columnFlex));
		compSections.push_back(std::move(compSection));
		satSections.push_back(std::move(satSection));
		gainSections.push_back(std::move(gainSection));
	}

	// Add band columns to the editor
	for (auto& col : bandColumns)
		addAndMakeVisible(col.get());

	// Limiter section
	limiterSection = std::make_unique<LimiterSection>(apvts);
	addAndMakeVisible(limiterSection.get());

	// Second logo section (left side)
	logoSectionLeft = std::make_unique<LogoSection>("https://github.com/mm-rnd/");
	addAndMakeVisible(logoSectionLeft.get());

	// Logo section (right side)
	logoSectionRight = std::make_unique<LogoSection>("http://www.morrell-rnd.co.uk");
	addAndMakeVisible(logoSectionRight.get());

	setSize(1000, 800);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
	// Reset look and feels before components are destroyed
	for (auto& col : bandColumns)
		col = nullptr;
	bandColumnLeftContainers.clear();
	bandColumnLeftFlexes.clear();
	compSections.clear();
	satSections.clear();
	gainSections.clear();
	limiterSection = nullptr;
	logoSectionLeft = nullptr;
	logoSectionRight = nullptr;
}

CompressorSection::Params AudioPluginAudioProcessorEditor::makeCompParams(const juce::String& prefix)
{
	CompressorSection::Params p;
	if (prefix == "Low")
	{
		p.attack = ParamUtils::toIdentifier(Param::LowAttack);
		p.release = ParamUtils::toIdentifier(Param::LowRelease);
		p.threshold = ParamUtils::toIdentifier(Param::LowThreshold);
		p.ratio = ParamUtils::toIdentifier(Param::LowRatio);
		p.knee = ParamUtils::toIdentifier(Param::LowKnee);
		p.lookahead = ParamUtils::toIdentifier(Param::LowLookahead);
		p.makeupGain = ParamUtils::toIdentifier(Param::LowMakeupGain);
	}
	else if (prefix == "Mid")
	{
		p.attack = ParamUtils::toIdentifier(Param::MidAttack);
		p.release = ParamUtils::toIdentifier(Param::MidRelease);
		p.threshold = ParamUtils::toIdentifier(Param::MidThreshold);
		p.ratio = ParamUtils::toIdentifier(Param::MidRatio);
		p.knee = ParamUtils::toIdentifier(Param::MidKnee);
		p.lookahead = ParamUtils::toIdentifier(Param::MidLookahead);
		p.makeupGain = ParamUtils::toIdentifier(Param::MidMakeupGain);
	}
	else
	{
		p.attack = ParamUtils::toIdentifier(Param::HighAttack);
		p.release = ParamUtils::toIdentifier(Param::HighRelease);
		p.threshold = ParamUtils::toIdentifier(Param::HighThreshold);
		p.ratio = ParamUtils::toIdentifier(Param::HighRatio);
		p.knee = ParamUtils::toIdentifier(Param::HighKnee);
		p.lookahead = ParamUtils::toIdentifier(Param::HighLookahead);
		p.makeupGain = ParamUtils::toIdentifier(Param::HighMakeupGain);
	}
	return p;
}

SaturationSection::Params AudioPluginAudioProcessorEditor::makeSatParams(const juce::String& prefix)
{
	SaturationSection::Params p;
	if (prefix == "Low")
	{
		p.drive = ParamUtils::toIdentifier(Param::LowSaturationDrive);
		p.evenOdd = ParamUtils::toIdentifier(Param::LowSaturationEvenOdd);
		p.heavy = ParamUtils::toIdentifier(Param::LowSaturationHeavy);
		p.mix = ParamUtils::toIdentifier(Param::LowSaturationMix);
	}
	else if (prefix == "Mid")
	{
		p.drive = ParamUtils::toIdentifier(Param::MidSaturationDrive);
		p.evenOdd = ParamUtils::toIdentifier(Param::MidSaturationEvenOdd);
		p.heavy = ParamUtils::toIdentifier(Param::MidSaturationHeavy);
		p.mix = ParamUtils::toIdentifier(Param::MidSaturationMix);
	}
	else
	{
		p.drive = ParamUtils::toIdentifier(Param::HighSaturationDrive);
		p.evenOdd = ParamUtils::toIdentifier(Param::HighSaturationEvenOdd);
		p.heavy = ParamUtils::toIdentifier(Param::HighSaturationHeavy);
		p.mix = ParamUtils::toIdentifier(Param::HighSaturationMix);
	}
	return p;
}

void AudioPluginAudioProcessorEditor::paint(juce::Graphics& g)
{
	if (analogueMode)
	{
		// Analogue mode: off-white matt background with depth and lighting from top-right
		auto bounds = getLocalBounds().toFloat();

		// Base off-white matt colour
		juce::Colour baseColour(0xFFE8E4DC);   // Warm off-white
		juce::Colour lightColour(0xFFF5F2EC);  // Lighter for highlights
		juce::Colour shadowColour(0xFFD4CFC5); // Darker for shadows

		// Create a radial gradient simulating light from top-right
		auto lightSource = bounds.getTopRight();
		auto gradient = juce::ColourGradient(lightColour, // Light colour at source
											 lightSource.getX() - bounds.getWidth() * 0.3f,
											 lightSource.getY() - bounds.getHeight() * 0.3f,
											 shadowColour, // Shadow colour at opposite corner
											 bounds.getBottomLeft().getX(), bounds.getBottomLeft().getY(), true);

		// Fill with gradient
		g.setGradientFill(gradient);
		g.fillRect(bounds);

		// Add subtle vignette effect for depth
		auto vignetteGradient = juce::ColourGradient(juce::Colours::transparentBlack, bounds.getCentreX(),
													 bounds.getCentreY(), juce::Colour(0xFF000000).withAlpha(0.08f),
													 bounds.getCentreX(), bounds.getCentreY(), false);
		vignetteGradient.addColour(0.0f, juce::Colours::transparentBlack);
		vignetteGradient.addColour(0.7f, juce::Colour(0xFF000000).withAlpha(0.05f));
		vignetteGradient.addColour(1.0f, juce::Colour(0xFF000000).withAlpha(0.12f));

		g.setGradientFill(vignetteGradient);
		g.fillRect(bounds);

		// Add subtle top highlight for 3D effect
		auto highlightGradient = juce::ColourGradient(
			juce::Colours::transparentWhite, bounds.getRight(), bounds.getY(), juce::Colours::transparentBlack,
			bounds.getRight() - bounds.getWidth() * 0.4f, bounds.getY() + bounds.getHeight() * 0.3f, true);
		highlightGradient.addColour(0.0f, juce::Colours::transparentWhite);
		highlightGradient.addColour(0.5f, juce::Colour(0xFFFFFFFF).withAlpha(0.15f));
		highlightGradient.addColour(1.0f, juce::Colours::transparentBlack);

		g.setGradientFill(highlightGradient);
		g.fillRect(bounds);
	}
	else
	{
		// Digital mode: original dark background
		g.fillAll(juce::Colour(0xFF0D0D1A));
	}
}

namespace
{
void updateSliderColoursForAnalogueMode(juce::Component& comp, bool analogueMode)
{
	// Walk all children recursively to find sliders
	if (auto* slider = dynamic_cast<juce::Slider*>(&comp))
	{
		if (analogueMode)
		{
			slider->setColour(juce::Slider::textBoxTextColourId, textBoxTextColourAnalogue);
			slider->setColour(juce::Slider::textBoxBackgroundColourId, textBoxBackgroundColourAnalogue);
			slider->setColour(juce::Slider::textBoxOutlineColourId, textBoxOutlineColourAnalogue);
		}
		else
		{
			slider->setColour(juce::Slider::textBoxTextColourId, textBoxTextColourDigital);
			slider->setColour(juce::Slider::textBoxBackgroundColourId, textBoxBackgroundColourDigital);
			slider->setColour(juce::Slider::textBoxOutlineColourId, textBoxOutlineColourDigital);
		}
	}

	// Walk all children recursively to find toggle buttons
	if (auto* toggle = dynamic_cast<ToggleComponent*>(&comp))
	{
		if (analogueMode)
			toggle->setTextColour(textBoxTextColourAnalogue);
		else
			toggle->setTextColour(textBoxTextColourDigital);
	}

	// Walk all children recursively to find labels
	if (auto* label = dynamic_cast<juce::Label*>(&comp))
	{
		const auto isNotTextBox = dynamic_cast<juce::Slider*>(label->getParentComponent()) == nullptr;

		if (analogueMode)
		{
			label->getProperties().set("originalText", label->getText());
			label->setColour(juce::Label::textColourId, textBoxTextColourAnalogue);
			if (isNotTextBox)
				label->setText(label->getText().toUpperCase(), juce::dontSendNotification);
		}
		else
		{
			const auto original = label->getProperties()["originalText"].toString();
			if (original.isNotEmpty() && isNotTextBox)
				label->setText(original, juce::dontSendNotification);
			label->setColour(juce::Label::textColourId, textBoxTextColourDigital);
		}
	}

	for (auto* child : comp.getChildren())
		updateSliderColoursForAnalogueMode(*child, analogueMode);
}
} // namespace

bool AudioPluginAudioProcessorEditor::keyPressed(const juce::KeyPress& key)
{
	// CMD+SHIFT+U on macOS, CTRL+SHIFT+U on Windows/Linux
	if ((key.getModifiers().isCommandDown() || key.getModifiers().isCtrlDown()) && key.getModifiers().isShiftDown() &&
		std::toupper(key.getKeyCode()) == 'U')
	{
		analogueMode = !analogueMode;

		// Update the static analogue mode flag in the LookAndFeel
		SectionLookAndFeel::setAnalogueMode(analogueMode);

		// Update all slider textbox colours recursively
		updateSliderColoursForAnalogueMode(*this, analogueMode);

		repaint(); // Trigger repaint to update background and all labels
		return true;
	}

	return false;
}

void AudioPluginAudioProcessorEditor::resized()
{
	auto area = getLocalBounds().reduced(8);

	// Audio analyser at top
	if (analyser)
		analyser->setBounds(area.removeFromTop(analyserHeight));

	area.removeFromTop(6);

	// Crossover section — full width
	crossoverSection.setBounds(area.removeFromTop(crossoverHeight));

	// Three band columns
	constexpr float limiterProportion = 0.875f; // 12.5% for limiter (half of previous 25%)
	auto bandsArea = area.removeFromTop(static_cast<int>(area.getHeight() * limiterProportion));

	// Layout each band column — pre-calculate equal width so all columns are the same size
	auto colWidth = bandsArea.getWidth() / static_cast<int>(bandColumns.size());
	for (size_t i = 0; i < bandColumns.size(); ++i)
	{
		auto colBounds = bandsArea.removeFromLeft(colWidth);
		bandColumns[i]->setBounds(colBounds);
		bandColumnFlexBoxes[i]->performLayout(bandColumns[i]->getLocalBounds());

		// Layout the left container (comp + sat) vertically
		if (i < bandColumnLeftContainers.size() && i < bandColumnLeftFlexes.size())
			bandColumnLeftFlexes[i]->performLayout(bandColumnLeftContainers[i]->getLocalBounds());
	}

	// Limiter section and logo sections at bottom
	if (limiterSection && logoSectionLeft && logoSectionRight)
	{
		// Calculate the size for the square logo section (same as limiter height)
		auto logoSize = area.getHeight();

		// Second logo takes the left square portion
		auto logoAreaLeft = area.removeFromLeft(logoSize);
		logoSectionLeft->setBounds(logoAreaLeft);

		// Logo takes the right square portion
		auto logoAreaRight = area.removeFromRight(logoSize);
		logoSectionRight->setBounds(logoAreaRight);

		// Limiter takes the remaining middle portion
		limiterSection->setBounds(area);
	}
}
