#include "ProgramManager.h"
#include <catch2/catch_test_macros.hpp>

// ---------------------------------------------------------------------------
//  ProgramData::createFactoryPrograms
// ---------------------------------------------------------------------------

TEST_CASE("ProgramData::createFactoryPrograms returns ten programs", "[programmanager]")
{
	const auto programs = ProgramData::createFactoryPrograms();

	SECTION("size is exactly 10")
	{
		CHECK(programs.size() == 10);
	}

	SECTION("each program has a non-empty name")
	{
		for (const auto& prog : programs)
			CHECK_FALSE(prog.name.isEmpty());
	}

	SECTION("all program names are distinct")
	{
		for (std::size_t i = 0; i < programs.size(); ++i)
			for (std::size_t j = i + 1; j < programs.size(); ++j)
				CHECK(programs[i].name != programs[j].name);
	}
}

TEST_CASE("ProgramData::createFactoryPrograms program names are correct", "[programmanager]")
{
	const auto programs = ProgramData::createFactoryPrograms();

	CHECK(programs[0].name == "Default");
	CHECK(programs[1].name == "Gentle Mastering");
	CHECK(programs[2].name == "Heavy Saturation");
	CHECK(programs[3].name == "Clean Limiting");
	CHECK(programs[4].name == "Aggressive Punch");
	CHECK(programs[5].name == "Warm Vinyl");
	CHECK(programs[6].name == "Modern Radio");
	CHECK(programs[7].name == "Bass Heavy Dub");
	CHECK(programs[8].name == "Bright EDM");
	CHECK(programs[9].name == "Lo-Fi Crush");
}

// ---------------------------------------------------------------------------
//  ProgramManager construction and accessors
// ---------------------------------------------------------------------------

TEST_CASE("ProgramManager constructor initialises state correctly", "[programmanager]")
{
	const ProgramManager mgr;

	SECTION("has exactly 10 programs")
	{
		CHECK(mgr.getNumPrograms() == 10);
	}

	SECTION("current program index starts at 0")
	{
		CHECK(mgr.getCurrentProgram() == 0);
	}
}

TEST_CASE("ProgramManager::getProgramName returns correct names", "[programmanager]")
{
	const ProgramManager mgr;

	CHECK(mgr.getProgramName(0) == "Default");
	CHECK(mgr.getProgramName(1) == "Gentle Mastering");
	CHECK(mgr.getProgramName(2) == "Heavy Saturation");
	CHECK(mgr.getProgramName(3) == "Clean Limiting");
	CHECK(mgr.getProgramName(4) == "Aggressive Punch");
	CHECK(mgr.getProgramName(5) == "Warm Vinyl");
	CHECK(mgr.getProgramName(6) == "Modern Radio");
	CHECK(mgr.getProgramName(7) == "Bass Heavy Dub");
	CHECK(mgr.getProgramName(8) == "Bright EDM");
	CHECK(mgr.getProgramName(9) == "Lo-Fi Crush");
}

TEST_CASE("ProgramManager::getProgramName clamps out-of-range indices", "[programmanager]")
{
	const ProgramManager mgr;

	SECTION("negative index returns program 0 name")
	{
		CHECK(mgr.getProgramName(-1) == "Default");
	}

	SECTION("index beyond last returns last program name")
	{
		CHECK(mgr.getProgramName(10) == "Lo-Fi Crush");
		CHECK(mgr.getProgramName(100) == "Lo-Fi Crush");
	}
}

TEST_CASE("ProgramManager::setProgramName modifies the correct program", "[programmanager]")
{
	ProgramManager mgr;

	mgr.setProgramName(2, "Custom Program");

	CHECK(mgr.getProgramName(2) == "Custom Program");

	// Other programs remain unchanged
	CHECK(mgr.getProgramName(1) == "Gentle Mastering");
	CHECK(mgr.getProgramName(3) == "Clean Limiting");
}

TEST_CASE("ProgramManager::setProgramName clamps out-of-range indices", "[programmanager]")
{
	ProgramManager mgr;

	SECTION("negative index modifies program 0")
	{
		mgr.setProgramName(-1, "Edited");
		CHECK(mgr.getProgramName(0) == "Edited");
	}

	SECTION("index beyond last modifies last program")
	{
		mgr.setProgramName(10, "Edited");
		CHECK(mgr.getProgramName(9) == "Edited");
	}
}

// ---------------------------------------------------------------------------
//  ProgramManager::getProgram
// ---------------------------------------------------------------------------

TEST_CASE("ProgramManager::getProgram returns correct program data", "[programmanager]")
{
	const ProgramManager mgr;

	const auto& def = mgr.getProgram(0);
	CHECK(def.name == "Default");
	CHECK(def.lowXoverDb == 120.0f);
	CHECK(def.highXoverDb == 8000.0f);
	CHECK(def.limiterCeiling == 0.0f);

	const auto& gentle = mgr.getProgram(1);
	CHECK(gentle.name == "Gentle Mastering");
	CHECK(gentle.lowThreshold == -12.0f);
	CHECK(gentle.limiterCeiling == -1.0f);

	const auto& heavy = mgr.getProgram(2);
	CHECK(heavy.name == "Heavy Saturation");
	CHECK(heavy.lowSatDrive == 60.0f);
	CHECK(heavy.limiterCeiling == -2.0f);
}

TEST_CASE("ProgramManager::getProgram clamps out-of-range indices", "[programmanager]")
{
	const ProgramManager mgr;

	SECTION("negative index returns program 0")
	{
		CHECK(mgr.getProgram(-1).name == "Default");
	}

	SECTION("index beyond last returns last program")
	{
		CHECK(mgr.getProgram(10).name == "Lo-Fi Crush");
		CHECK(mgr.getProgram(100).name == "Lo-Fi Crush");
	}
}

// ---------------------------------------------------------------------------
//  ProgramManager::syncCurrentProgramIndex
// ---------------------------------------------------------------------------

TEST_CASE("ProgramManager::syncCurrentProgramIndex updates the index", "[programmanager]")
{
	ProgramManager mgr;

	mgr.syncCurrentProgramIndex(5);
	CHECK(mgr.getCurrentProgram() == 5);
}

TEST_CASE("ProgramManager::syncCurrentProgramIndex clamps out-of-range values", "[programmanager]")
{
	ProgramManager mgr;

	SECTION("negative index is clamped to 0")
	{
		mgr.syncCurrentProgramIndex(-5);
		CHECK(mgr.getCurrentProgram() == 0);
	}

	SECTION("index beyond last is clamped to last index (9)")
	{
		mgr.syncCurrentProgramIndex(15);
		CHECK(mgr.getCurrentProgram() == 9);
	}
}
