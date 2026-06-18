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

	SECTION("LowSaturationDrive")
	{
		CHECK(ParamUtils::toIdentifier(Param::LowSaturationDrive) == "LowSaturationDrive");
	}

	SECTION("LowSaturationEvenOdd")
	{
		CHECK(ParamUtils::toIdentifier(Param::LowSaturationEvenOdd) == "LowSaturationEvenOdd");
	}

	SECTION("LowSaturationHeavy")
	{
		CHECK(ParamUtils::toIdentifier(Param::LowSaturationHeavy) == "LowSaturationHeavy");
	}

	SECTION("LowSaturationMix")
	{
		CHECK(ParamUtils::toIdentifier(Param::LowSaturationMix) == "LowSaturationMix");
	}

	SECTION("MidSaturationDrive")
	{
		CHECK(ParamUtils::toIdentifier(Param::MidSaturationDrive) == "MidSaturationDrive");
	}

	SECTION("MidSaturationEvenOdd")
	{
		CHECK(ParamUtils::toIdentifier(Param::MidSaturationEvenOdd) == "MidSaturationEvenOdd");
	}

	SECTION("MidSaturationHeavy")
	{
		CHECK(ParamUtils::toIdentifier(Param::MidSaturationHeavy) == "MidSaturationHeavy");
	}

	SECTION("MidSaturationMix")
	{
		CHECK(ParamUtils::toIdentifier(Param::MidSaturationMix) == "MidSaturationMix");
	}

	SECTION("HighSaturationDrive")
	{
		CHECK(ParamUtils::toIdentifier(Param::HighSaturationDrive) == "HighSaturationDrive");
	}

	SECTION("HighSaturationEvenOdd")
	{
		CHECK(ParamUtils::toIdentifier(Param::HighSaturationEvenOdd) == "HighSaturationEvenOdd");
	}

	SECTION("HighSaturationHeavy")
	{
		CHECK(ParamUtils::toIdentifier(Param::HighSaturationHeavy) == "HighSaturationHeavy");
	}

	SECTION("HighSaturationMix")
	{
		CHECK(ParamUtils::toIdentifier(Param::HighSaturationMix) == "HighSaturationMix");
	}

	SECTION("LowOutputGain")
	{
		CHECK(ParamUtils::toIdentifier(Param::LowOutputGain) == "LowOutputGain");
	}

	SECTION("MidOutputGain")
	{
		CHECK(ParamUtils::toIdentifier(Param::MidOutputGain) == "MidOutputGain");
	}

	SECTION("HighOutputGain")
	{
		CHECK(ParamUtils::toIdentifier(Param::HighOutputGain) == "HighOutputGain");
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

	SECTION("LowSaturationDrive")
	{
		CHECK(ParamUtils::toName(Param::LowSaturationDrive) == "Low Sat Drive");
	}

	SECTION("LowSaturationEvenOdd")
	{
		CHECK(ParamUtils::toName(Param::LowSaturationEvenOdd) == "Low Sat Even/Odd");
	}

	SECTION("LowSaturationHeavy")
	{
		CHECK(ParamUtils::toName(Param::LowSaturationHeavy) == "Low Sat Heavy");
	}

	SECTION("LowSaturationMix")
	{
		CHECK(ParamUtils::toName(Param::LowSaturationMix) == "Low Sat Mix");
	}

	SECTION("MidSaturationDrive")
	{
		CHECK(ParamUtils::toName(Param::MidSaturationDrive) == "Mid Sat Drive");
	}

	SECTION("MidSaturationEvenOdd")
	{
		CHECK(ParamUtils::toName(Param::MidSaturationEvenOdd) == "Mid Sat Even/Odd");
	}

	SECTION("MidSaturationHeavy")
	{
		CHECK(ParamUtils::toName(Param::MidSaturationHeavy) == "Mid Sat Heavy");
	}

	SECTION("MidSaturationMix")
	{
		CHECK(ParamUtils::toName(Param::MidSaturationMix) == "Mid Sat Mix");
	}

	SECTION("HighSaturationDrive")
	{
		CHECK(ParamUtils::toName(Param::HighSaturationDrive) == "High Sat Drive");
	}

	SECTION("HighSaturationEvenOdd")
	{
		CHECK(ParamUtils::toName(Param::HighSaturationEvenOdd) == "High Sat Even/Odd");
	}

	SECTION("HighSaturationHeavy")
	{
		CHECK(ParamUtils::toName(Param::HighSaturationHeavy) == "High Sat Heavy");
	}

	SECTION("HighSaturationMix")
	{
		CHECK(ParamUtils::toName(Param::HighSaturationMix) == "High Sat Mix");
	}

	SECTION("LowOutputGain")
	{
		CHECK(ParamUtils::toName(Param::LowOutputGain) == "Low Output");
	}

	SECTION("MidOutputGain")
	{
		CHECK(ParamUtils::toName(Param::MidOutputGain) == "Mid Output");
	}

	SECTION("HighOutputGain")
	{
		CHECK(ParamUtils::toName(Param::HighOutputGain) == "High Output");
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
		{Param::LowSaturationDrive, "LowSaturationDrive"},
		{Param::LowSaturationEvenOdd, "LowSaturationEvenOdd"},
		{Param::LowSaturationHeavy, "LowSaturationHeavy"},
		{Param::LowSaturationMix, "LowSaturationMix"},
		{Param::MidSaturationDrive, "MidSaturationDrive"},
		{Param::MidSaturationEvenOdd, "MidSaturationEvenOdd"},
		{Param::MidSaturationHeavy, "MidSaturationHeavy"},
		{Param::MidSaturationMix, "MidSaturationMix"},
		{Param::HighSaturationDrive, "HighSaturationDrive"},
		{Param::HighSaturationEvenOdd, "HighSaturationEvenOdd"},
		{Param::HighSaturationHeavy, "HighSaturationHeavy"},
		{Param::HighSaturationMix, "HighSaturationMix"},
		{Param::LowOutputGain, "LowOutputGain"},
		{Param::MidOutputGain, "MidOutputGain"},
		{Param::HighOutputGain, "HighOutputGain"},
	};

	for (const auto& [param, expectedID] : cases)
	{
		const auto pid = ParamUtils::toParameterID(param);
		CHECK(pid.getParamID() == juce::String(expectedID));
	}
}
