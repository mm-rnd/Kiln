#include "ProgramManager.h"

//==============================================================================
// Helpers
//==============================================================================

/** Reads a float parameter from APVTS and returns its value, or fallback if missing. */
static float readFloatParam(juce::AudioProcessorValueTreeState& apvts, Param param, float fallback) noexcept
{
	const auto* ptr = apvts.getRawParameterValue(ParamUtils::toIdentifier(param));
	return ptr != nullptr ? ptr->load() : fallback;
}

/** Reads a choice/ratio parameter from APVTS and returns its index, or fallback if missing. */
static int readChoiceParam(juce::AudioProcessorValueTreeState& apvts, Param param, int fallback) noexcept
{
	const auto* p = apvts.getParameter(ParamUtils::toIdentifier(param));
	if (auto* choice = dynamic_cast<const juce::AudioParameterChoice*>(p))
		return choice->getIndex();
	return fallback;
}

/** Reads a bool parameter from APVTS and returns its value, or fallback if missing. */
static bool readBoolParam(juce::AudioProcessorValueTreeState& apvts, Param param, bool fallback) noexcept
{
	const auto* p = apvts.getParameter(ParamUtils::toIdentifier(param));
	if (auto* b = dynamic_cast<const juce::AudioParameterBool*>(p))
		return b->get();
	return fallback;
}

/** Sets a float APVTS parameter to a new value (normalised range is handled internally). */
static void setFloatParam(juce::AudioProcessorValueTreeState& apvts, Param param, float value)
{
	auto* p = apvts.getParameter(ParamUtils::toIdentifier(param));
	if (p != nullptr)
		p->setValueNotifyingHost(p->convertTo0to1(value));
}

/** Sets a choice APVTS parameter index. */
static void setChoiceParam(juce::AudioProcessorValueTreeState& apvts, Param param, int index)
{
	auto* p = apvts.getParameter(ParamUtils::toIdentifier(param));
	if (p != nullptr)
	{
		const auto norm = p->convertTo0to1(static_cast<float>(index));
		p->setValueNotifyingHost(norm);
	}
}

/** Sets a bool APVTS parameter. */
static void setBoolParam(juce::AudioProcessorValueTreeState& apvts, Param param, bool value)
{
	auto* p = apvts.getParameter(ParamUtils::toIdentifier(param));
	if (auto* b = dynamic_cast<juce::AudioParameterBool*>(p))
		b->setValueNotifyingHost(value ? 1.0f : 0.0f);
}

//==============================================================================
// Program
//==============================================================================

void ProgramData::readFromAPVTS(juce::AudioProcessorValueTreeState& apvts)
{
	// Crossover
	lowXoverDb = readFloatParam(apvts, Param::LowXoverDb, lowXoverDb);
	highXoverDb = readFloatParam(apvts, Param::HighXoverDb, highXoverDb);

	// Low compressor
	lowAttack = readFloatParam(apvts, Param::LowAttack, lowAttack);
	lowRelease = readFloatParam(apvts, Param::LowRelease, lowRelease);
	lowThreshold = readFloatParam(apvts, Param::LowThreshold, lowThreshold);
	lowRatio = readChoiceParam(apvts, Param::LowRatio, lowRatio);
	lowKnee = readBoolParam(apvts, Param::LowKnee, lowKnee);
	lowLookahead = readBoolParam(apvts, Param::LowLookahead, lowLookahead);
	lowMakeupGain = readFloatParam(apvts, Param::LowMakeupGain, lowMakeupGain);

	// Mid compressor
	midAttack = readFloatParam(apvts, Param::MidAttack, midAttack);
	midRelease = readFloatParam(apvts, Param::MidRelease, midRelease);
	midThreshold = readFloatParam(apvts, Param::MidThreshold, midThreshold);
	midRatio = readChoiceParam(apvts, Param::MidRatio, midRatio);
	midKnee = readBoolParam(apvts, Param::MidKnee, midKnee);
	midLookahead = readBoolParam(apvts, Param::MidLookahead, midLookahead);
	midMakeupGain = readFloatParam(apvts, Param::MidMakeupGain, midMakeupGain);

	// High compressor
	highAttack = readFloatParam(apvts, Param::HighAttack, highAttack);
	highRelease = readFloatParam(apvts, Param::HighRelease, highRelease);
	highThreshold = readFloatParam(apvts, Param::HighThreshold, highThreshold);
	highRatio = readChoiceParam(apvts, Param::HighRatio, highRatio);
	highKnee = readBoolParam(apvts, Param::HighKnee, highKnee);
	highLookahead = readBoolParam(apvts, Param::HighLookahead, highLookahead);
	highMakeupGain = readFloatParam(apvts, Param::HighMakeupGain, highMakeupGain);

	// Low saturation
	lowSatDrive = readFloatParam(apvts, Param::LowSaturationDrive, lowSatDrive);
	lowSatEvenOdd = readFloatParam(apvts, Param::LowSaturationEvenOdd, lowSatEvenOdd);
	lowSatHeavy = readBoolParam(apvts, Param::LowSaturationHeavy, lowSatHeavy);
	lowSatMix = readFloatParam(apvts, Param::LowSaturationMix, lowSatMix);

	// Mid saturation
	midSatDrive = readFloatParam(apvts, Param::MidSaturationDrive, midSatDrive);
	midSatEvenOdd = readFloatParam(apvts, Param::MidSaturationEvenOdd, midSatEvenOdd);
	midSatHeavy = readBoolParam(apvts, Param::MidSaturationHeavy, midSatHeavy);
	midSatMix = readFloatParam(apvts, Param::MidSaturationMix, midSatMix);

	// High saturation
	highSatDrive = readFloatParam(apvts, Param::HighSaturationDrive, highSatDrive);
	highSatEvenOdd = readFloatParam(apvts, Param::HighSaturationEvenOdd, highSatEvenOdd);
	highSatHeavy = readBoolParam(apvts, Param::HighSaturationHeavy, highSatHeavy);
	highSatMix = readFloatParam(apvts, Param::HighSaturationMix, highSatMix);

	// Output gains
	lowOutputGain = readFloatParam(apvts, Param::LowOutputGain, lowOutputGain);
	midOutputGain = readFloatParam(apvts, Param::MidOutputGain, midOutputGain);
	highOutputGain = readFloatParam(apvts, Param::HighOutputGain, highOutputGain);

	// Limiter
	limiterCeiling = readFloatParam(apvts, Param::LimiterCeiling, limiterCeiling);
}

void ProgramData::applyToAPVTS(juce::AudioProcessorValueTreeState& apvts) const
{
	// Crossover
	setFloatParam(apvts, Param::LowXoverDb, lowXoverDb);
	setFloatParam(apvts, Param::HighXoverDb, highXoverDb);

	// Low compressor
	setFloatParam(apvts, Param::LowAttack, lowAttack);
	setFloatParam(apvts, Param::LowRelease, lowRelease);
	setFloatParam(apvts, Param::LowThreshold, lowThreshold);
	setChoiceParam(apvts, Param::LowRatio, lowRatio);
	setBoolParam(apvts, Param::LowKnee, lowKnee);
	setBoolParam(apvts, Param::LowLookahead, lowLookahead);
	setFloatParam(apvts, Param::LowMakeupGain, lowMakeupGain);

	// Mid compressor
	setFloatParam(apvts, Param::MidAttack, midAttack);
	setFloatParam(apvts, Param::MidRelease, midRelease);
	setFloatParam(apvts, Param::MidThreshold, midThreshold);
	setChoiceParam(apvts, Param::MidRatio, midRatio);
	setBoolParam(apvts, Param::MidKnee, midKnee);
	setBoolParam(apvts, Param::MidLookahead, midLookahead);
	setFloatParam(apvts, Param::MidMakeupGain, midMakeupGain);

	// High compressor
	setFloatParam(apvts, Param::HighAttack, highAttack);
	setFloatParam(apvts, Param::HighRelease, highRelease);
	setFloatParam(apvts, Param::HighThreshold, highThreshold);
	setChoiceParam(apvts, Param::HighRatio, highRatio);
	setBoolParam(apvts, Param::HighKnee, highKnee);
	setBoolParam(apvts, Param::HighLookahead, highLookahead);
	setFloatParam(apvts, Param::HighMakeupGain, highMakeupGain);

	// Low saturation
	setFloatParam(apvts, Param::LowSaturationDrive, lowSatDrive);
	setFloatParam(apvts, Param::LowSaturationEvenOdd, lowSatEvenOdd);
	setBoolParam(apvts, Param::LowSaturationHeavy, lowSatHeavy);
	setFloatParam(apvts, Param::LowSaturationMix, lowSatMix);

	// Mid saturation
	setFloatParam(apvts, Param::MidSaturationDrive, midSatDrive);
	setFloatParam(apvts, Param::MidSaturationEvenOdd, midSatEvenOdd);
	setBoolParam(apvts, Param::MidSaturationHeavy, midSatHeavy);
	setFloatParam(apvts, Param::MidSaturationMix, midSatMix);

	// High saturation
	setFloatParam(apvts, Param::HighSaturationDrive, highSatDrive);
	setFloatParam(apvts, Param::HighSaturationEvenOdd, highSatEvenOdd);
	setBoolParam(apvts, Param::HighSaturationHeavy, highSatHeavy);
	setFloatParam(apvts, Param::HighSaturationMix, highSatMix);

	// Output gains
	setFloatParam(apvts, Param::LowOutputGain, lowOutputGain);
	setFloatParam(apvts, Param::MidOutputGain, midOutputGain);
	setFloatParam(apvts, Param::HighOutputGain, highOutputGain);

	// Limiter
	setFloatParam(apvts, Param::LimiterCeiling, limiterCeiling);
}

std::array<ProgramData, 10> ProgramData::createFactoryPrograms()
{
	std::array<ProgramData, 10> programs{};

	// --- Program 0: Default / Flat ---
	programs[0].name = "Default";
	// All member initialisers already match the default constructor values.

	// --- Program 1: Gentle Mastering ---
	programs[1].name = "Gentle Mastering";
	programs[1].lowAttack = 20.0f;
	programs[1].lowRelease = 200.0f;
	programs[1].lowThreshold = -12.0f;
	programs[1].lowRatio = 1; // 2:1
	programs[1].lowKnee = true;
	programs[1].lowMakeupGain = 3.0f;
	programs[1].midAttack = 20.0f;
	programs[1].midRelease = 200.0f;
	programs[1].midThreshold = -12.0f;
	programs[1].midRatio = 1; // 2:1
	programs[1].midKnee = true;
	programs[1].midMakeupGain = 3.0f;
	programs[1].highAttack = 15.0f;
	programs[1].highRelease = 150.0f;
	programs[1].highThreshold = -10.0f;
	programs[1].highRatio = 1; // 2:1
	programs[1].highKnee = true;
	programs[1].highMakeupGain = 2.0f;
	programs[1].limiterCeiling = -1.0f;

	// --- Program 2: Heavy Saturation ---
	programs[2].name = "Heavy Saturation";
	programs[2].lowThreshold = -30.0f;
	programs[2].lowRatio = 3; // 8:1
	programs[2].lowMakeupGain = 6.0f;
	programs[2].midThreshold = -30.0f;
	programs[2].midRatio = 3; // 8:1
	programs[2].midMakeupGain = 6.0f;
	programs[2].highThreshold = -30.0f;
	programs[2].highRatio = 3; // 8:1
	programs[2].highMakeupGain = 6.0f;
	programs[2].lowSatDrive = 60.0f;
	programs[2].lowSatEvenOdd = 50.0f;
	programs[2].lowSatMix = 40.0f;
	programs[2].midSatDrive = 60.0f;
	programs[2].midSatEvenOdd = 50.0f;
	programs[2].midSatMix = 40.0f;
	programs[2].highSatDrive = 50.0f;
	programs[2].highSatEvenOdd = 30.0f;
	programs[2].highSatMix = 30.0f;
	programs[2].limiterCeiling = -2.0f;

	// --- Program 3: Clean Limiting ---
	programs[3].name = "Clean Limiting";
	programs[3].lowThreshold = -6.0f;
	programs[3].lowRatio = 5; // 20:1 — brickwall
	programs[3].lowAttack = 2.0f;
	programs[3].lowRelease = 50.0f;
	programs[3].lowKnee = false;
	programs[3].midThreshold = -6.0f;
	programs[3].midRatio = 5; // 20:1
	programs[3].midAttack = 2.0f;
	programs[3].midRelease = 50.0f;
	programs[3].midKnee = false;
	programs[3].highThreshold = -6.0f;
	programs[3].highRatio = 5; // 20:1
	programs[3].highAttack = 1.0f;
	programs[3].highRelease = 30.0f;
	programs[3].highKnee = false;
	programs[3].limiterCeiling = -0.5f;

	// --- Program 4: Aggressive Punch ---
	programs[4].name = "Aggressive Punch";
	programs[4].lowAttack = 5.0f;
	programs[4].lowRelease = 80.0f;
	programs[4].lowThreshold = -20.0f;
	programs[4].lowRatio = 4; // 12:1
	programs[4].lowMakeupGain = 8.0f;
	programs[4].midAttack = 8.0f;
	programs[4].midRelease = 120.0f;
	programs[4].midThreshold = -18.0f;
	programs[4].midRatio = 3; // 8:1
	programs[4].midMakeupGain = 6.0f;
	programs[4].highAttack = 3.0f;
	programs[4].highRelease = 60.0f;
	programs[4].highThreshold = -22.0f;
	programs[4].highRatio = 5; // 20:1
	programs[4].highMakeupGain = 4.0f;
	programs[4].lowSatDrive = 80.0f;
	programs[4].lowSatEvenOdd = 70.0f;
	programs[4].lowSatHeavy = true;
	programs[4].lowSatMix = 60.0f;
	programs[4].midSatDrive = 60.0f;
	programs[4].midSatEvenOdd = 40.0f;
	programs[4].midSatMix = 50.0f;
	programs[4].highSatDrive = 40.0f;
	programs[4].highSatEvenOdd = 20.0f;
	programs[4].highSatMix = 30.0f;
	programs[4].lowOutputGain = -6.0f;
	programs[4].midOutputGain = -4.0f;
	programs[4].highOutputGain = -2.0f;
	programs[4].limiterCeiling = -1.5f;

	// --- Program 5: Warm Vinyl ---
	// Wide crossover, soft compression on lows + warm saturation, subtle limiting
	programs[5].name = "Warm Vinyl";
	programs[5].lowXoverDb = 250.0f;
	programs[5].highXoverDb = 10000.0f;
	programs[5].lowAttack = 30.0f;
	programs[5].lowRelease = 250.0f;
	programs[5].lowThreshold = -14.0f;
	programs[5].lowRatio = 1; // 2:1
	programs[5].lowKnee = true;
	programs[5].lowMakeupGain = 2.0f;
	programs[5].midAttack = 15.0f;
	programs[5].midRelease = 120.0f;
	programs[5].midThreshold = -18.0f;
	programs[5].midRatio = 2; // 4:1
	programs[5].midKnee = true;
	programs[5].midMakeupGain = 2.5f;
	programs[5].highAttack = 10.0f;
	programs[5].highRelease = 80.0f;
	programs[5].highThreshold = -20.0f;
	programs[5].highRatio = 2; // 4:1
	programs[5].highKnee = true;
	programs[5].highMakeupGain = 3.0f;
	programs[5].lowSatDrive = 30.0f;
	programs[5].lowSatEvenOdd = 60.0f; // more even harmonics for warmth
	programs[5].lowSatMix = 25.0f;
	programs[5].midSatDrive = 20.0f;
	programs[5].midSatEvenOdd = 40.0f;
	programs[5].midSatMix = 20.0f;
	programs[5].highSatDrive = 10.0f;
	programs[5].highSatEvenOdd = 30.0f;
	programs[5].highSatMix = 15.0f;
	programs[5].lowOutputGain = -1.0f;
	programs[5].midOutputGain = -1.5f;
	programs[5].highOutputGain = -2.0f;
	programs[5].limiterCeiling = -0.8f;

	// --- Program 6: Modern Radio ---
	// Tight aggressive compression with soft knee to avoid clicking, lots of saturation character, heavy limiting
	programs[6].name = "Modern Radio";
	programs[6].lowXoverDb = 100.0f;
	programs[6].highXoverDb = 6000.0f;
	programs[6].lowAttack = 8.0f;
	programs[6].lowRelease = 80.0f;
	programs[6].lowThreshold = -22.0f;
	programs[6].lowRatio = 4; // 12:1
	programs[6].lowKnee = true; // soft knee prevents hard clamping clicks
	programs[6].lowLookahead = true;
	programs[6].lowMakeupGain = 6.0f;
	programs[6].midAttack = 10.0f;
	programs[6].midRelease = 100.0f;
	programs[6].midThreshold = -20.0f;
	programs[6].midRatio = 3; // 8:1
	programs[6].midKnee = true; // soft knee prevents hard clamping clicks
	programs[6].midLookahead = true;
	programs[6].midMakeupGain = 5.0f;
	programs[6].highAttack = 6.0f;
	programs[6].highRelease = 60.0f;
	programs[6].highThreshold = -24.0f;
	programs[6].highRatio = 5; // 20:1
	programs[6].highKnee = true; // soft knee prevents hard clamping clicks
	programs[6].highLookahead = true;
	programs[6].highMakeupGain = 4.0f;
	programs[6].lowSatDrive = 50.0f;
	programs[6].lowSatEvenOdd = 20.0f;
	programs[6].lowSatMix = 40.0f;
	programs[6].midSatDrive = 45.0f;
	programs[6].midSatEvenOdd = 10.0f;
	programs[6].midSatMix = 35.0f;
	programs[6].highSatDrive = 55.0f;
	programs[6].highSatEvenOdd = -20.0f; // lean odd harmonics on highs for edge
	programs[6].highSatHeavy = true;
	programs[6].highSatMix = 50.0f;
	programs[6].lowOutputGain = -4.0f;
	programs[6].midOutputGain = -3.0f;
	programs[6].highOutputGain = -5.0f;
	programs[6].limiterCeiling = -0.3f;

	// --- Program 7: Bass Heavy Dub ---
	// Wide low band, light compression on lows, heavy limiting, focus on low-end saturation
	programs[7].name = "Bass Heavy Dub";
	programs[7].lowXoverDb = 400.0f; // wide low band
	programs[7].highXoverDb = 5000.0f;
	programs[7].lowAttack = 50.0f;
	programs[7].lowRelease = 300.0f;
	programs[7].lowThreshold = -8.0f;
	programs[7].lowRatio = 1; // 2:1 gentle
	programs[7].lowKnee = true;
	programs[7].lowMakeupGain = 1.0f;
	programs[7].midAttack = 20.0f;
	programs[7].midRelease = 150.0f;
	programs[7].midThreshold = -16.0f;
	programs[7].midRatio = 2; // 4:1
	programs[7].midKnee = true;
	programs[7].midMakeupGain = 4.0f;
	programs[7].highAttack = 10.0f;
	programs[7].highRelease = 100.0f;
	programs[7].highThreshold = -18.0f;
	programs[7].highRatio = 3; // 8:1
	programs[7].highKnee = false;
	programs[7].highMakeupGain = 5.0f;
	programs[7].lowSatDrive = 70.0f;
	programs[7].lowSatEvenOdd = 80.0f; // mostly even, subby warmth
	programs[7].lowSatHeavy = true;
	programs[7].lowSatMix = 60.0f;
	programs[7].midSatDrive = 30.0f;
	programs[7].midSatEvenOdd = 50.0f;
	programs[7].midSatMix = 20.0f;
	programs[7].highSatDrive = 15.0f;
	programs[7].highSatEvenOdd = 0.0f;
	programs[7].highSatMix = 10.0f;
	programs[7].lowOutputGain = -3.0f;
	programs[7].midOutputGain = -2.0f;
	programs[7].highOutputGain = -4.0f;
	programs[7].limiterCeiling = -0.5f;

	// --- Program 8: Bright EDM ---
	// Narrow bands, fast compression on highs, clean saturation, loud ceiling
	programs[8].name = "Bright EDM";
	programs[8].lowXoverDb = 200.0f;
	programs[8].highXoverDb = 12000.0f; // extend high band for air
	programs[8].lowAttack = 8.0f;
	programs[8].lowRelease = 100.0f;
	programs[8].lowThreshold = -16.0f;
	programs[8].lowRatio = 2; // 4:1
	programs[8].lowKnee = true;
	programs[8].lowMakeupGain = 4.0f;
	programs[8].midAttack = 6.0f;
	programs[8].midRelease = 80.0f;
	programs[8].midThreshold = -14.0f;
	programs[8].midRatio = 2; // 4:1
	programs[8].midKnee = true;
	programs[8].midMakeupGain = 3.0f;
	programs[8].highAttack = 2.0f;
	programs[8].highRelease = 40.0f;
	programs[8].highThreshold = -12.0f;
	programs[8].highRatio = 2; // 4:1 gentle
	programs[8].highKnee = true;
	programs[8].highMakeupGain = 2.0f;
	programs[8].lowSatDrive = 25.0f;
	programs[8].lowSatEvenOdd = 30.0f;
	programs[8].lowSatMix = 20.0f;
	programs[8].midSatDrive = 35.0f;
	programs[8].midSatEvenOdd = 0.0f; // balanced harmonics
	programs[8].midSatHeavy = true;
	programs[8].midSatMix = 25.0f;
	programs[8].highSatDrive = 20.0f;
	programs[8].highSatEvenOdd = -50.0f; // odd harmonics for sparkle
	programs[8].highSatMix = 30.0f;
	programs[8].lowOutputGain = -2.0f;
	programs[8].midOutputGain = -1.0f;
	programs[8].highOutputGain = 0.0f;
	programs[8].limiterCeiling = -0.1f;

	// --- Program 9: Lo-Fi Crush ---
	// Aggressive compression with soft knee to avoid clicking, heavy saturation,
	// narrow bands. Uses less extreme makeup gain to avoid over-compression pumping.
	programs[9].name = "Lo-Fi Crush";
	programs[9].lowXoverDb = 80.0f;
	programs[9].highXoverDb = 4000.0f;
	programs[9].lowAttack = 5.0f;
	programs[9].lowRelease = 80.0f;
	programs[9].lowThreshold = -30.0f;
	programs[9].lowRatio = 5; // 20:1
	programs[9].lowKnee = true; // soft knee prevents hard clamping clicks
	programs[9].lowLookahead = true;
	programs[9].lowMakeupGain = 8.0f;
	programs[9].midAttack = 8.0f;
	programs[9].midRelease = 100.0f;
	programs[9].midThreshold = -28.0f;
	programs[9].midRatio = 4; // 12:1
	programs[9].midKnee = true; // soft knee prevents hard clamping clicks
	programs[9].midLookahead = true;
	programs[9].midMakeupGain = 6.0f;
	programs[9].highAttack = 5.0f;
	programs[9].highRelease = 60.0f;
	programs[9].highThreshold = -32.0f;
	programs[9].highRatio = 5; // 20:1
	programs[9].highKnee = true; // soft knee prevents hard clamping clicks
	programs[9].highLookahead = true;
	programs[9].highMakeupGain = 6.0f;
	programs[9].lowSatDrive = 70.0f;
	programs[9].lowSatEvenOdd = 50.0f;
	programs[9].lowSatHeavy = true;
	programs[9].lowSatMix = 60.0f;
	programs[9].midSatDrive = 65.0f;
	programs[9].midSatEvenOdd = 30.0f;
	programs[9].midSatHeavy = true;
	programs[9].midSatMix = 55.0f;
	programs[9].highSatDrive = 75.0f;
	programs[9].highSatEvenOdd = -70.0f; // odd harmonics for harsh crunch
	programs[9].highSatHeavy = true;
	programs[9].highSatMix = 70.0f;
	programs[9].lowOutputGain = -8.0f;
	programs[9].midOutputGain = -6.0f;
	programs[9].highOutputGain = -10.0f;
	programs[9].limiterCeiling = -3.0f;

	return programs;
}

//==============================================================================
// ProgramManager
//==============================================================================

ProgramManager::ProgramManager() : programs(ProgramData::createFactoryPrograms()) {}

const juce::String& ProgramManager::getProgramName(int index) const
{
	const auto idx = static_cast<std::size_t>(juce::jlimit(0, getNumPrograms() - 1, index));
	return programs[idx].name;
}

void ProgramManager::setProgramName(int index, const juce::String& newName)
{
	const auto idx = static_cast<std::size_t>(juce::jlimit(0, getNumPrograms() - 1, index));
	programs[idx].name = newName;
}

void ProgramManager::setCurrentProgram(int index, juce::AudioProcessorValueTreeState& apvts)
{
	const auto idx = static_cast<std::size_t>(juce::jlimit(0, getNumPrograms() - 1, index));
	currentProgram = static_cast<int>(idx);
	programs[idx].applyToAPVTS(apvts);
}

const ProgramData& ProgramManager::getProgram(int index) const
{
	const auto idx = static_cast<std::size_t>(juce::jlimit(0, getNumPrograms() - 1, index));
	return programs[idx];
}