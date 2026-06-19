#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "ParamUtils.h"

#include <array>
#include <cstddef>

//==============================================================================
/**
 * Represents a single program (preset) containing values for every parameter
 * exposed by the plug-in, plus a human-readable name.
 */
struct ProgramData
{
	/** Human-readable name for this program. */
	juce::String name;

	// --- Crossover ---
	float lowXoverDb = 120.0f;
	float highXoverDb = 8000.0f;

	// --- Low compressor ---
	float lowAttack = 10.0f;
	float lowRelease = 100.0f;
	float lowThreshold = -24.0f;
	int lowRatio = 2; // index into ratioChoices (4:1)
	bool lowKnee = true;
	bool lowLookahead = false;
	float lowMakeupGain = 0.0f;

	// --- Mid compressor ---
	float midAttack = 10.0f;
	float midRelease = 100.0f;
	float midThreshold = -24.0f;
	int midRatio = 2;
	bool midKnee = true;
	bool midLookahead = false;
	float midMakeupGain = 0.0f;

	// --- High compressor ---
	float highAttack = 10.0f;
	float highRelease = 100.0f;
	float highThreshold = -24.0f;
	int highRatio = 2;
	bool highKnee = true;
	bool highLookahead = false;
	float highMakeupGain = 0.0f;

	// --- Low saturation ---
	float lowSatDrive = 0.0f;
	float lowSatEvenOdd = 0.0f;
	bool lowSatHeavy = false;
	float lowSatMix = 0.0f;

	// --- Mid saturation ---
	float midSatDrive = 0.0f;
	float midSatEvenOdd = 0.0f;
	bool midSatHeavy = false;
	float midSatMix = 0.0f;

	// --- High saturation ---
	float highSatDrive = 0.0f;
	float highSatEvenOdd = 0.0f;
	bool highSatHeavy = false;
	float highSatMix = 0.0f;

	// --- Output gains ---
	float lowOutputGain = 0.0f;
	float midOutputGain = 0.0f;
	float highOutputGain = 0.0f;

	// --- Limiter ---
	float limiterCeiling = 0.0f;

	//==============================================================================
	/** Read a float value from the APVTS and store it in the appropriate member. */
	void readFromAPVTS(juce::AudioProcessorValueTreeState& apvts);

	/** Write all stored values into the APVTS parameters. */
	void applyToAPVTS(juce::AudioProcessorValueTreeState& apvts) const;

	//==============================================================================
	/** Return a collection of ten factory-default programs. */
	static std::array<ProgramData, 10> createFactoryPrograms();
};

//==============================================================================
/**
 * Manages a fixed set of programs for the audio processor, providing the
 * interface expected by juce::AudioProcessor's program-related methods.
 */
class ProgramManager
{
  public:
	ProgramManager();

	//--------------------------------------------------------------------------
	// Accessors
	//--------------------------------------------------------------------------
	[[nodiscard]] int getNumPrograms() const noexcept { return static_cast<int>(programs.size()); }
	[[nodiscard]] int getCurrentProgram() const noexcept { return currentProgram; }
	[[nodiscard]] const juce::String& getProgramName(int index) const;
	void setProgramName(int index, const juce::String& newName);

	//--------------------------------------------------------------------------
	/** Load a program by index and apply its values to the APVTS. */
	void setCurrentProgram(int index, juce::AudioProcessorValueTreeState& apvts);

	/**
	 * Synchronise the internal current program index without modifying APVTS.
	 * Used when restoring state where the APVTS has already been populated.
	 */
	void syncCurrentProgramIndex(int index) { currentProgram = juce::jlimit(0, getNumPrograms() - 1, index); }

	//--------------------------------------------------------------------------
	/** Return the program at the given index (read-only). */
	[[nodiscard]] const ProgramData& getProgram(int index) const;

  private:
	std::array<ProgramData, 10> programs;
	int currentProgram = 0;
};