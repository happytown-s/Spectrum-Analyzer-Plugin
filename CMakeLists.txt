cmake_minimum_required(VERSION 3.22)
project(SpectrumAnalyzer VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# JUCE path - can be overridden with -DJUCE_PATH=...
if(NOT DEFINED JUCE_PATH)
    set(JUCE_PATH "$ENV{HOME}/JUCE")
endif()

# Verify JUCE exists
if(NOT EXISTS "${JUCE_PATH}/CMakeLists.txt")
    message(FATAL_ERROR "JUCE not found at ${JUCE_PATH}. Please specify -DJUCE_PATH=/path/to/JUCE")
endif()

# Add JUCE
add_subdirectory("${JUCE_PATH}" "${CMAKE_BINARY_DIR}/JUCE")

# Define JUCE Plugin
juce_add_plugin(SpectrumAnalyzer
    COMPANY_NAME "YourCompany"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE Manu
    PLUGIN_CODE Spec
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "Spectrum Analyzer"
)

# Source files
target_sources(SpectrumAnalyzer PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/SpectrumAnalyzerComponent.cpp
)

# Include directories
target_include_directories(SpectrumAnalyzer PRIVATE
    Source
)

# Compile definitions
target_compile_definitions(SpectrumAnalyzer PUBLIC
    JUCE_STRICT_REFCOUNTEDPOINTER=1
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

# Link JUCE modules
target_link_libraries(SpectrumAnalyzer PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_plugin_client
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_core
    juce::juce_data_structures
    juce::juce_dsp
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)
