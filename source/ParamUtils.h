#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

/** Enumerates every automatable parameter exposed by the plug-in. */
enum class Param
{
	LowXoverDb,
	HighXoverDb,
	LowAttack,
	LowRelease,
	LowThreshold,
	LowRatio,
	LowKnee,
	LowLookahead,
	LowMakeupGain,
	MidAttack,
	MidRelease,
	MidThreshold,
	MidRatio,
	MidKnee,
	MidLookahead,
	MidMakeupGain,
	HighAttack,
	HighRelease,
	HighThreshold,
	HighRatio,
	HighKnee,
	HighLookahead,
	HighMakeupGain,
};

namespace ParamUtils
{
/** Convert a Param enum value to its APVTS identifier string. */
[[nodiscard]] juce::String toIdentifier(Param param);

/** Convert a Param enum value to its human-readable display name. */
[[nodiscard]] juce::String toName(Param param);

/** Return the ParameterID (identifier + version) used when building the APVTS layout. */
[[nodiscard]] juce::ParameterID toParameterID(Param param, int versionHint = 1);

} // namespace ParamUtils