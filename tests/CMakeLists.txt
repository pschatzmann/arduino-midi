# Unit tests: run with "ctest" in the build directory
set(MIDI_TESTS
    test_midi_parser
    test_midi_ble_parser
    test_midi_common
    test_midi_callback_action
    test_midi_logger
)

# The Stream based classes can only be tested with the Arduino Emulator
if(TARGET arduino_emulator)
    list(APPEND MIDI_TESTS test_midi_stream)
else()
    message(STATUS "arduino-midi: arduino_emulator not available - skipping test_midi_stream")
endif()

foreach(test_name ${MIDI_TESTS})
    add_executable(${test_name} ${test_name}.cpp)
    target_link_libraries(${test_name} PRIVATE arduino-midi)
    target_compile_options(${test_name} PRIVATE -Wall -Wextra -Wno-unused-parameter)
    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()
