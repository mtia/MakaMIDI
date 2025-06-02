cmake_minimum_required(VERSION 3.15)

project(MakaMIDI VERSION 1.0.0)

add_subdirectory(C:/Dev/JUCE JUCE_BUILD)

# Enable JUCE GUI application features
set(JUCE_ENABLE_MODULE_SOURCE_GROUPS ON)

# Add AudioPluginHost 
#add_subdirectory(C:/Dev/JUCE/extras/AudioPluginHost AudioPluginHost)

set(VST3_OUTPUT_DIR "C:\\VstPlugins\\MakaMIDI" CACHE PATH "Output directory for VST3 plugin")


# Create an audio plugin
juce_add_plugin(MakaMIDI
    PLUGIN_MANUFACTURER_CODE Maka
    PLUGIN_CODE Maka
    FORMATS VST3                     # The formats to build
    PRODUCT_NAME "MakaMIDI"
    COMPANY_NAME "YourCompany"
    VERSION "1.0.0"
    # VST3 Categories: "Fx"|"Instrument"|"Analyzer"|"Delay"|"Distortion" etc.
    VST3_CATEGORIES "Instrument"    # Make sure this is "Instrument"
    IS_MIDI_EFFECT TRUE            # Add this line
    NEEDS_MIDI_INPUT TRUE          # Add this line
    NEEDS_MIDI_OUTPUT TRUE         # Add this line if you output MIDI
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE    # Add this if you need keyboard input
)

# Add your source files
target_sources(MakaMIDI
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/LowBox.h
        Source/MidiProcessor.h
        Source/NoteAlteration.h
)

target_compile_definitions(MakaMIDI
    PUBLIC
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_MODAL_LOOPS_PERMITTED=1
)

target_link_libraries(MakaMIDI
    PRIVATE
        MakaMIDI_Resources
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_audio_plugin_client
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
)

target_include_directories(MakaMIDI
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
        ${JUCE_MODULES_DIR}
)

# Add binary resources
juce_add_binary_data(MakaMIDI_Resources
    SOURCES
        Oud.png    # Point directly to the file in root
)

add_custom_command(TARGET MakaMIDI_VST3 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${VST3_OUTPUT_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_BINARY_DIR}/MakaMIDI_artefacts/Debug/VST3/MakaMIDI.vst3"
        "${VST3_OUTPUT_DIR}/MakaMIDI.vst3"
)


add_custom_target(copy_vst3 ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_BINARY_DIR}/MakaMIDI_artefacts/Debug/VST3/MakaMIDI.vst3/Contents/x86_64-win/MakaMIDI.vst3"
        "${VST3_OUTPUT_DIR}/MakaMIDI.vst3"
    DEPENDS MakaMIDI
)


