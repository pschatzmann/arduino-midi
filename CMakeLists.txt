cmake_minimum_required(VERSION 3.16)

project(arduino-midi LANGUAGES CXX)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(MIDI_IS_TOP_LEVEL ON)
else()
    set(MIDI_IS_TOP_LEVEL OFF)
endif()

option(MIDI_BUILD_TESTS "Build the arduino-midi unit tests" ${MIDI_IS_TOP_LEVEL})
option(MIDI_USE_ARDUINO_EMULATOR "Build the Stream based classes with the Arduino Emulator" OFF)
set(ARDUINO_EMULATOR_DIR "" CACHE PATH "Local Arduino-Emulator checkout (otherwise it is downloaded)")

if(MIDI_USE_ARDUINO_EMULATOR AND NOT TARGET arduino_emulator)
    if(ARDUINO_EMULATOR_DIR)
        add_subdirectory(${ARDUINO_EMULATOR_DIR} ${CMAKE_CURRENT_BINARY_DIR}/arduino_emulator)
    else()
        include(FetchContent)
        FetchContent_Declare(arduino_emulator
            GIT_REPOSITORY "https://github.com/pschatzmann/Arduino-Emulator.git"
            GIT_TAG main)
        FetchContent_MakeAvailable(arduino_emulator)
    endif()
endif()

if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

# Platform independent core: parser, message generation and logging.
# This part does not need Arduino.
add_library(arduino-midi STATIC
    src/MidiParser.cpp
    src/MidiCommon.cpp
    src/MidiLogger.cpp
)
target_include_directories(arduino-midi PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_options(arduino-midi PRIVATE -Wall -Wno-format)

# The Stream based classes need an Arduino API implementation
# (e.g. the arduino-emulator target "arduino_emulator")
if(TARGET arduino_emulator)
    target_sources(arduino-midi PRIVATE
        src/MidiStreamIn.cpp
        src/MidiStreamOut.cpp
    )
    target_link_libraries(arduino-midi PUBLIC arduino_emulator)
endif()

add_library(arduino-midi::arduino-midi ALIAS arduino-midi)

install(TARGETS arduino-midi ARCHIVE DESTINATION lib)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src/"
    DESTINATION "include/arduino-midi"
    FILES_MATCHING PATTERN "*.h")

if(MIDI_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
