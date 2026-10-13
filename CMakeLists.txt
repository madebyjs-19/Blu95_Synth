cmake_minimum_required(VERSION 3.22)

set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum macOS version")

project(Blu95Synth VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# JUCE is downloaded automatically. To use a local copy instead, configure with
#   -DFETCHCONTENT_SOURCE_DIR_JUCE=/path/to/JUCE
include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.9
    GIT_SHALLOW    TRUE)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(Blu95Synth
    PRODUCT_NAME                "Blu95 FM"
    COMPANY_NAME                "madebyjs"
    BUNDLE_ID                   com.madebyjs.blu95fm
    PLUGIN_MANUFACTURER_CODE    Mbjs
    PLUGIN_CODE                 Bl95
    IS_SYNTH                    TRUE
    NEEDS_MIDI_INPUT            TRUE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD     FALSE
    VST3_CATEGORIES             Instrument Synth
    AU_MAIN_TYPE                kAudioUnitType_MusicDevice
    FORMATS                     VST3 AU Standalone)

target_sources(Blu95Synth PRIVATE
    Source/Parameters.cpp
    Source/Presets.cpp
    Source/FMVoice.cpp
    Source/InfoText.cpp
    Source/RetroLookAndFeel.cpp
    Source/Components.cpp
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp)

target_compile_definitions(Blu95Synth PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

if (MSVC)
    # Source files contain Korean text in UTF-8.
    target_compile_options(Blu95Synth PRIVATE /utf-8)
endif()

target_link_libraries(Blu95Synth
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
