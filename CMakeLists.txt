cmake_minimum_required(VERSION 3.22)

project(SALEK_HIGHTECH VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# JUCE location
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/JUCE/CMakeLists.txt")
    add_subdirectory(JUCE)
elseif (EXISTS "/tmp/juce-src/CMakeLists.txt")
    add_subdirectory(/tmp/juce-src JUCE)
else()
    message(FATAL_ERROR "JUCE not found. Clone into ./JUCE (CI) or /tmp/juce-src")
endif()

juce_add_plugin(SALEK_HIGHTECH
    COMPANY_NAME "SALEK"
    BUNDLE_ID com.salek.hightech
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE Sale
    PLUGIN_CODE Htec
    FORMATS VST3 Standalone
    PRODUCT_NAME "SALEK HIGHTECH"
    VST3_CATEGORIES "Instrument Synth"
)

target_sources(SALEK_HIGHTECH
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/Synth/SynthEngine.cpp
        Source/Synth/SynthEngine.h
        Source/Synth/SynthVoice.cpp
        Source/Synth/SynthVoice.h
        Source/Synth/SynthSound.cpp
        Source/Synth/SynthSound.h
        Source/Oscillators/WavetableOscillator.cpp
        Source/Oscillators/WavetableOscillator.h
        Source/Wavetables/WavetableData.cpp
        Source/Wavetables/WavetableData.h
        Source/Filters/StateVariableFilter.h
        Source/Modulation/LFO.h
        Source/Modulation/ModMatrix.h
        Source/FX/SimpleDelay.h
        Source/Sequencer/Arpeggiator.h
        Source/Sequencer/StepSequencer.h
        Source/DSP/SmoothedValue.h
)

target_compile_definitions(SALEK_HIGHTECH
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_STRICT_REFCOUNTEDPOINTER=1
)

target_link_libraries(SALEK_HIGHTECH
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

juce_generate_juce_header(SALEK_HIGHTECH)

# Helpful for Windows CI
if (MSVC)
    target_compile_options(SALEK_HIGHTECH PRIVATE /W3 /MP)
endif()
