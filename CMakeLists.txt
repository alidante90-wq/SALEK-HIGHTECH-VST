cmake_minimum_required(VERSION 3.22)
project(GITI_BY_SALEK_HIGHTECH VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# SALEK UNIVERSE / GITI GENESIS
# Every build is a real, independently identified Genesis instrument.
set(GITI_EDITION_ID "1" CACHE STRING "GITI Genesis edition number (1..50)")
if (NOT GITI_EDITION_ID MATCHES "^[0-9]+$")
    set(GITI_EDITION_ID "1")
endif()
if (GITI_EDITION_ID LESS 1 OR GITI_EDITION_ID GREATER 50)
    message(FATAL_ERROR "GITI_EDITION_ID must be between 1 and 50")
endif()

if (GITI_EDITION_ID LESS 10)
    set(GITI_EDITION_PADDED "00${GITI_EDITION_ID}")
elseif (GITI_EDITION_ID LESS 100)
    set(GITI_EDITION_PADDED "0${GITI_EDITION_ID}")
else()
    set(GITI_EDITION_PADDED "${GITI_EDITION_ID}")
endif()

# VST3 plugin codes must be exactly four characters: G001 ... G050.
set(GITI_PLUGIN_CODE "G${GITI_EDITION_PADDED}")
option(GITI_NEMO_MODE "Build NEMOGITI locked demo edition" OFF)
set(GITI_PRODUCT_NAME "GITI BY SALEK HIGHTECH GENESIS ${GITI_EDITION_PADDED}")
set(GITI_BUNDLE_ID "com.salek.giti.genesis.${GITI_EDITION_ID}")

if (GITI_NEMO_MODE)
    set(GITI_PLUGIN_CODE "NEMO")
    set(GITI_PRODUCT_NAME "NEMOGITI BY SALEK HIGHTECH")
    set(GITI_BUNDLE_ID "com.salek.nemogiti")
endif()

if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/JUCE/CMakeLists.txt")
    add_subdirectory(JUCE)
else()
    include(FetchContent)
    FetchContent_Declare(
        JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.4
    )
    FetchContent_MakeAvailable(JUCE)
endif()

juce_add_plugin(GITI_BY_SALEK_HIGHTECH
    COMPANY_NAME "Salek"
    BUNDLE_ID "${GITI_BUNDLE_ID}"
    PLUGIN_MANUFACTURER_CODE Salk
    PLUGIN_CODE "${GITI_PLUGIN_CODE}"
    PLUGIN_NAME "${GITI_PRODUCT_NAME}"
    PRODUCT_NAME "${GITI_PRODUCT_NAME}"
    DESCRIPTION "GITI Genesis sonic machine • SALEK Universe"
    FORMATS VST3 Standalone
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES "Instrument Synth"
)

juce_generate_juce_header(GITI_BY_SALEK_HIGHTECH)

file(GLOB SALEK_ICON_ASSETS CONFIGURE_DEPENDS "Source/Assets/icons/*.png")

juce_add_binary_data(SalekBinaryAssets SOURCES
    "Source/Assets/bg_isatis.jpg"
    "Source/Assets/bg_salek.jpg"
    "Source/Assets/logo_giti.png"
    "Source/Assets/logo_giti_alt.png"
    "Source/Assets/icons_atlas.png"
    "Source/Assets/logo_salek_sheet.png"
    "Source/Assets/char_toronowla.png"
    "Source/Assets/char_neon_street.png"
    "Source/Assets/char_pink_dead.png"
    "Source/Assets/char_apron.png"
    "Source/Assets/char_catgirl.png"
    "Source/Assets/char_cyber_white.png"
    "Source/Assets/char_foxgirl.png"
    "Source/Assets/char_gun.png"
    "Source/Assets/char_purple_latex.png"
    "Source/Assets/char_white_suit.png"
    ${SALEK_ICON_ASSETS}
)

target_sources(GITI_BY_SALEK_HIGHTECH PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
    Source/AssetsData.h
    Source/GitiEdition.h
    Source/GitiSonicDNA.h
    Source/GitiLore.h
    Source/Synth/SynthEngine.cpp
    Source/Synth/SynthEngine.h
    Source/Synth/SynthVoice.cpp
    Source/Synth/SynthVoice.h
    Source/Synth/SynthSound.cpp
    Source/Synth/SynthSound.h
    Source/Oscillators/WavetableOscillator.cpp
    Source/Oscillators/WavetableOscillator.h
    Source/Oscillators/GranularOscillator.h
    Source/Wavetables/WavetableData.cpp
    Source/Wavetables/WavetableData.h
    Source/Filters/StateVariableFilter.h
    Source/Modulation/LFO.h
    Source/Modulation/ModMatrix.h
    Source/FX/SimpleDelay.h
    Source/FX/SimpleChorus.h
    Source/FX/SimpleReverb.h
    Source/FX/SimpleCompressor.h
    Source/FX/SimpleEQ.h
    Source/FX/SimpleSpatial.h
    Source/FX/SimplePhaser.h
    Source/FX/SimpleDistortion.h
    Source/FX/MagicEngine.h
    Source/Sequencer/Arpeggiator.h
    Source/Sequencer/StepSequencer.h
    Source/DSP/SmoothedValue.h
    Source/SHAE/SHAECore.h
    Source/UI/ModMatrixPanel.h
)

target_include_directories(GITI_BY_SALEK_HIGHTECH PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Source)

target_compile_definitions(GITI_BY_SALEK_HIGHTECH PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    GITI_EDITION_ID=${GITI_EDITION_ID}
    GITI_NEMO_MODE=$<BOOL:${GITI_NEMO_MODE}>
)

target_link_libraries(GITI_BY_SALEK_HIGHTECH
    PRIVATE juce::juce_audio_utils juce::juce_dsp juce::juce_opengl juce::juce_cryptography SalekBinaryAssets
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags juce::juce_recommended_warning_flags
)

if (MSVC)
    target_compile_options(GITI_BY_SALEK_HIGHTECH PRIVATE /W3 /MP /bigobj /utf-8)
endif()

add_custom_command(TARGET GITI_BY_SALEK_HIGHTECH POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/Source/Assets"
        "$<TARGET_FILE_DIR:GITI_BY_SALEK_HIGHTECH>/Assets"
    COMMENT "Copy GITI Universe assets beside edition binary"
)

juce_add_console_app(GITI_SHAE_SELFTEST PRODUCT_NAME "GITI SHAE Self Test")
target_sources(GITI_SHAE_SELFTEST PRIVATE Source/Tests/SHAESelfTest.cpp)
target_link_libraries(GITI_SHAE_SELFTEST PRIVATE
    juce::juce_audio_basics
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags
)
