cmake_minimum_required(VERSION 3.23.1)

# Catch2 unit tests
add_subdirectory(${CMAKE_SOURCE_DIR}/modules/Catch2 ${CMAKE_BINARY_DIR}/Catch2)

enable_testing()

add_executable(KilnTests
    LinearPhaseCrossover.cpp
    ParamUtils.cpp
)

target_link_libraries(KilnTests PRIVATE
    SharedCode
    Catch2::Catch2WithMain
    juce::juce_audio_basics
    juce::juce_audio_processors
    juce::juce_core
    juce::juce_dsp
)

target_sources(KilnTests PRIVATE
    ${CMAKE_SOURCE_DIR}/source/LinearPhaseCrossover.cpp
    ${CMAKE_SOURCE_DIR}/source/ParamUtils.cpp
)

add_test(NAME KilnTests COMMAND KilnTests)

message(STATUS "Kiln: tests enabled")