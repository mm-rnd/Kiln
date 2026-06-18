#include "ParamUtils.h"
#include <catch2/catch_test_macros.hpp>

// ---------------------------------------------------------------------------
//  toIdentifier
// ---------------------------------------------------------------------------

TEST_CASE("ParamUtils::toIdentifier returns the correct identifier string", "[paramutils]")
{
	SECTION("LowXoverDb")
	{
		CHECK(ParamUtils::toIdentifier(Param::LowXoverDb) == "LowXoverDb");
	}

	SECTION("HighXoverDb")
	{
		CHECK(ParamUtils::toIdentifier(Param::HighXoverDb) == "HighXoverDb");
	}

	SECTION("LowAttack")
	{
		CHECK(ParamUtils::toIdentifier(Param::LowAttack) == "LowAttack");
	}

	SECTION("LowRelease")
	{
		CHECK(ParamUtils::toIdentifier(Param::LowRelease) == "LowRelease");
	}

	SECTION("LowThreshold")
	{
		CHECK(ParamUtils::toIdentifier(Param::LowThreshold) == "LowThreshold");
	}

	SECTION("LowRatio")
	{
		CHECK(ParamUtils::toIdentifier(Param::LowRatio) == "LowRatio");
	}

	SECTION("LowKnee")
	{
		CHECK(ParamUtils::toIdentifier(Param::LowKnee) == "LowKnee");
	}

	SECTION("LowLookahead")
	{
		CHECK(ParamUtils::toIdentifier(Param::LowLookahead) == "LowLookahead");
	}

	SECTION("LowMakeupGain")
	{
		CHECK(ParamUtils::toIdentifier(Param::LowMakeupGain) == "LowMakeupGain");
	}

	SECTION("MidAttack")
	{
		CHECK(ParamUtils::toIdentifier(Param::MidAttack) == "MidAttack");
	}

	SECTION("MidRelease")
	{
		CHECK(ParamUtils::toIdentifier(Param::MidRelease) == "MidRelease");
	}

	SECTION("MidThreshold")
	{
		CHECK(ParamUtils::toIdentifier(Param::MidThreshold) == "MidThreshold");
	}

	SECTION("MidRatio")
	{
		CHECK(ParamUtils::toIdentifier(Param::MidRatio) == "MidRatio");
	}

	SECTION("MidKnee")
	{
		CHECK(ParamUtils::toIdentifier(Param::MidKnee) == "MidKnee");
	}

	SECTION("MidLookahead")
	{
		CHECK(ParamUtils::toIdentifier(Param::MidLookahead) == "MidLookahead");
	}

	SECTION("MidMakeupGain")
	{
		CHECK(ParamUtils::toIdentifier(Param::MidMakeupGain) == "MidMakeupGain");
	}

	SECTION("HighAttack")
	{
		CHECK(ParamUtils::toIdentifier(Param::HighAttack) == "HighAttack");
	}

	SECTION("HighRelease")
	{
		CHECK(ParamUtils::toIdentifier(Param::HighRelease) == "HighRelease");
	}

	SECTION("HighThreshold")
	{
		CHECK(ParamUtils::toIdentifier(Param::HighThreshold) == "HighThreshold");
	}

	SECTION("HighRatio")
	{
		CHECK(ParamUtils::toIdentifier(Param::HighRatio) == "HighRatio");
	}

	SECTION("HighKnee")
	{
		CHECK(ParamUtils::toIdentifier(Param::HighKnee) == "HighKnee");
	}

	SECTION("HighLookahead")
	{
		CHECK(ParamUtils::toIdentifier(Param::HighLookahead) == "HighLookahead");
	}

	SECTION("HighMakeupGain")
	{
		CHECK(ParamUtils::toIdentifier(Param::HighMakeupGain) == "HighMakeupGain");
	}
}

// ---------------------------------------------------------------------------
//  toName
// ---------------------------------------------------------------------------

TEST_CASE("ParamUtils::toName returns the correct display name", "[paramutils]")
{
	SECTION("LowXoverDb")
	{
		CHECK(ParamUtils::toName(Param::LowXoverDb) == "Low Xover");
	}

	SECTION("HighXoverDb")
	{
		CHECK(ParamUtils::toName(Param::HighXoverDb) == "High Xover");
	}

	SECTION("LowAttack")
	{
		CHECK(ParamUtils::toName(Param::LowAttack) == "Low Attack");
	}

	SECTION("LowRelease")
	{
		CHECK(ParamUtils::toName(Param::LowRelease) == "Low Release");
	}

	SECTION("LowThreshold")
	{
		CHECK(ParamUtils::toName(Param::LowThreshold) == "Low Threshold");
	}

	SECTION("LowRatio")
	{
		CHECK(ParamUtils::toName(Param::LowRatio) == "Low Ratio");
	}

	SECTION("LowKnee")
	{
		CHECK(ParamUtils::toName(Param::LowKnee) == "Low Knee");
	}

	SECTION("LowLookahead")
	{
		CHECK(ParamUtils::toName(Param::LowLookahead) == "Low Lookahead");
	}

	SECTION("LowMakeupGain")
	{
		CHECK(ParamUtils::toName(Param::LowMakeupGain) == "Low Makeup Gain");
	}

	SECTION("MidAttack")
	{
		CHECK(ParamUtils::toName(Param::MidAttack) == "Mid Attack");
	}

	SECTION("MidRelease")
	{
		CHECK(ParamUtils::toName(Param::MidRelease) == "Mid Release");
	}

	SECTION("MidThreshold")
	{
		CHECK(ParamUtils::toName(Param::MidThreshold) == "Mid Threshold");
	}

	SECTION("MidRatio")
	{
		CHECK(ParamUtils::toName(Param::MidRatio) == "Mid Ratio");
	}

	SECTION("MidKnee")
	{
		CHECK(ParamUtils::toName(Param::MidKnee) == "Mid Knee");
	}

	SECTION("MidLookahead")
	{
		CHECK(ParamUtils::toName(Param::MidLookahead) == "Mid Lookahead");
	}

	SECTION("MidMakeupGain")
	{
		CHECK(ParamUtils::toName(Param::MidMakeupGain) == "Mid Makeup Gain");
	}

	SECTION("HighAttack")
	{
		CHECK(ParamUtils::toName(Param::HighAttack) == "High Attack");
	}

	SECTION("HighRelease")
	{
		CHECK(ParamUtils::toName(Param::HighRelease) == "High Release");
	}

	SECTION("HighThreshold")
	{
		CHECK(ParamUtils::toName(Param::HighThreshold) == "High Threshold");
	}

	SECTION("HighRatio")
	{
		CHECK(ParamUtils::toName(Param::HighRatio) == "High Ratio");
	}

	SECTION("HighKnee")
	{
		CHECK(ParamUtils::toName(Param::HighKnee) == "High Knee");
	}

	SECTION("HighLookahead")
	{
		CHECK(ParamUtils::toName(Param::HighLookahead) == "High Lookahead");
	}

	SECTION("HighMakeupGain")
	{
		CHECK(ParamUtils::toName(Param::HighMakeupGain) == "High Makeup Gain");
	}
}

// ---------------------------------------------------------------------------
//  toParameterID
// ---------------------------------------------------------------------------

TEST_CASE("ParamUtils::toParameterID returns a correct ParameterID", "[paramutils]")
{
	SECTION("uses the identifier from toIdentifier")
	{
		const auto id = ParamUtils::toParameterID(Param::LowXoverDb);
		CHECK(id.getParamID() == ParamUtils::toIdentifier(Param::LowXoverDb));
	}

	SECTION("uses the supplied versionHint")
	{
		const auto id = ParamUtils::toParameterID(Param::HighXoverDb, 2);
		CHECK(id.getParamID() == "HighXoverDb");
		CHECK(id.getVersionHint() == 2);
	}

	SECTION("defaults versionHint to 1")
	{
		const auto id = ParamUtils::toParameterID(Param::LowXoverDb);
		CHECK(id.getVersionHint() == 1);
	}
}

// ---------------------------------------------------------------------------
//  Round-trip: identifier ↔ ParameterID
// ---------------------------------------------------------------------------

TEST_CASE("ParamUtils: ParameterID round-trip matches the enum", "[paramutils]")
{
	constexpr std::pair<Param, const char*> cases[] = {
		{Param::LowXoverDb, "LowXoverDb"},
		{Param::HighXoverDb, "HighXoverDb"},
		{Param::LowAttack, "LowAttack"},
		{Param::LowRelease, "LowRelease"},
		{Param::LowThreshold, "LowThreshold"},
		{Param::LowRatio, "LowRatio"},
		{Param::LowKnee, "LowKnee"},
		{Param::LowLookahead, "LowLookahead"},
		{Param::LowMakeupGain, "LowMakeupGain"},
		{Param::MidAttack, "MidAttack"},
		{Param::MidRelease, "MidRelease"},
		{Param::MidThreshold, "MidThreshold"},
		{Param::MidRatio, "MidRatio"},
		{Param::MidKnee, "MidKnee"},
		{Param::MidLookahead, "MidLookahead"},
		{Param::MidMakeupGain, "MidMakeupGain"},
		{Param::HighAttack, "HighAttack"},
		{Param::HighRelease, "HighRelease"},
		{Param::HighThreshold, "HighThreshold"},
		{Param::HighRatio, "HighRatio"},
		{Param::HighKnee, "HighKnee"},
		{Param::HighLookahead, "HighLookahead"},
		{Param::HighMakeupGain, "HighMakeupGain"},
	};

	for (const auto& [param, expectedID] : cases)
	{
		const auto pid = ParamUtils::toParameterID(param);
		CHECK(pid.getParamID() == juce::String(expectedID));
	}
}
