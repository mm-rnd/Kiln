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
	};

	for (const auto& [param, expectedID] : cases)
	{
		const auto pid = ParamUtils::toParameterID(param);
		CHECK(pid.getParamID() == juce::String(expectedID));
	}
}