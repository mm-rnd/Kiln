#include "ParamUtils.h"

juce::String ParamUtils::toIdentifier(Param param)
{
	switch (param)
	{
	case Param::LowXoverDb:
		return "LowXoverDb";
	case Param::HighXoverDb:
		return "HighXoverDb";
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
	}
	jassertfalse;
	return {};
}

juce::ParameterID ParamUtils::toParameterID(Param param, int versionHint)
{
	return {toIdentifier(param), versionHint};
}