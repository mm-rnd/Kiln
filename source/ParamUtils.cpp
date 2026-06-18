#include "ParamUtils.h"

juce::String ParamUtils::toIdentifier(Param param)
{
	switch (param)
	{
	case Param::LowXoverDb:
		return "LowXoverDb";
	case Param::HighXoverDb:
		return "HighXoverDb";
	case Param::LowAttack:
		return "LowAttack";
	case Param::LowRelease:
		return "LowRelease";
	case Param::LowThreshold:
		return "LowThreshold";
	case Param::LowRatio:
		return "LowRatio";
	case Param::LowKnee:
		return "LowKnee";
	case Param::LowLookahead:
		return "LowLookahead";
	case Param::LowMakeupGain:
		return "LowMakeupGain";
	case Param::MidAttack:
		return "MidAttack";
	case Param::MidRelease:
		return "MidRelease";
	case Param::MidThreshold:
		return "MidThreshold";
	case Param::MidRatio:
		return "MidRatio";
	case Param::MidKnee:
		return "MidKnee";
	case Param::MidLookahead:
		return "MidLookahead";
	case Param::MidMakeupGain:
		return "MidMakeupGain";
	case Param::HighAttack:
		return "HighAttack";
	case Param::HighRelease:
		return "HighRelease";
	case Param::HighThreshold:
		return "HighThreshold";
	case Param::HighRatio:
		return "HighRatio";
	case Param::HighKnee:
		return "HighKnee";
	case Param::HighLookahead:
		return "HighLookahead";
	case Param::HighMakeupGain:
		return "HighMakeupGain";
	case Param::LowSaturationDrive:
		return "LowSaturationDrive";
	case Param::LowSaturationEvenOdd:
		return "LowSaturationEvenOdd";
	case Param::LowSaturationHeavy:
		return "LowSaturationHeavy";
	case Param::LowSaturationMix:
		return "LowSaturationMix";
	case Param::MidSaturationDrive:
		return "MidSaturationDrive";
	case Param::MidSaturationEvenOdd:
		return "MidSaturationEvenOdd";
	case Param::MidSaturationHeavy:
		return "MidSaturationHeavy";
	case Param::MidSaturationMix:
		return "MidSaturationMix";
	case Param::HighSaturationDrive:
		return "HighSaturationDrive";
	case Param::HighSaturationEvenOdd:
		return "HighSaturationEvenOdd";
	case Param::HighSaturationHeavy:
		return "HighSaturationHeavy";
	case Param::HighSaturationMix:
		return "HighSaturationMix";
	case Param::LowOutputGain:
		return "LowOutputGain";
	case Param::MidOutputGain:
		return "MidOutputGain";
	case Param::HighOutputGain:
		return "HighOutputGain";
	}
	jassertfalse;
	return {};
}

juce::String ParamUtils::toName(Param param)
{
	switch (param)
	{
	case Param::LowXoverDb:
		return "Low Xover";
	case Param::HighXoverDb:
		return "High Xover";
	case Param::LowAttack:
		return "Low Attack";
	case Param::LowRelease:
		return "Low Release";
	case Param::LowThreshold:
		return "Low Threshold";
	case Param::LowRatio:
		return "Low Ratio";
	case Param::LowKnee:
		return "Low Knee";
	case Param::LowLookahead:
		return "Low Lookahead";
	case Param::LowMakeupGain:
		return "Low Makeup Gain";
	case Param::MidAttack:
		return "Mid Attack";
	case Param::MidRelease:
		return "Mid Release";
	case Param::MidThreshold:
		return "Mid Threshold";
	case Param::MidRatio:
		return "Mid Ratio";
	case Param::MidKnee:
		return "Mid Knee";
	case Param::MidLookahead:
		return "Mid Lookahead";
	case Param::MidMakeupGain:
		return "Mid Makeup Gain";
	case Param::HighAttack:
		return "High Attack";
	case Param::HighRelease:
		return "High Release";
	case Param::HighThreshold:
		return "High Threshold";
	case Param::HighRatio:
		return "High Ratio";
	case Param::HighKnee:
		return "High Knee";
	case Param::HighLookahead:
		return "High Lookahead";
	case Param::HighMakeupGain:
		return "High Makeup Gain";
	case Param::LowSaturationDrive:
		return "Low Sat Drive";
	case Param::LowSaturationEvenOdd:
		return "Low Sat Even/Odd";
	case Param::LowSaturationHeavy:
		return "Low Sat Heavy";
	case Param::LowSaturationMix:
		return "Low Sat Mix";
	case Param::MidSaturationDrive:
		return "Mid Sat Drive";
	case Param::MidSaturationEvenOdd:
		return "Mid Sat Even/Odd";
	case Param::MidSaturationHeavy:
		return "Mid Sat Heavy";
	case Param::MidSaturationMix:
		return "Mid Sat Mix";
	case Param::HighSaturationDrive:
		return "High Sat Drive";
	case Param::HighSaturationEvenOdd:
		return "High Sat Even/Odd";
	case Param::HighSaturationHeavy:
		return "High Sat Heavy";
	case Param::HighSaturationMix:
		return "High Sat Mix";
	case Param::LowOutputGain:
		return "Low Output";
	case Param::MidOutputGain:
		return "Mid Output";
	case Param::HighOutputGain:
		return "High Output";
	}
	jassertfalse;
	return {};
}

juce::ParameterID ParamUtils::toParameterID(Param param, int versionHint)
{
	return {toIdentifier(param), versionHint};
}