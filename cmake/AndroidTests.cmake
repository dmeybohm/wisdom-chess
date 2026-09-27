# Runs the tests of an Android build on the connected device or emulator.
#
# android-run-test.sh becomes CMAKE_CROSSCOMPILING_EMULATOR, which test
# discovery and ctest put in front of every test program. Discovery runs the
# programs while building, so the build needs a device as much as ctest does.
#
# Include after the test options are declared and before any test target.

if(NOT ANDROID OR CMAKE_CROSSCOMPILING_EMULATOR)
    return()
endif()

if(NOT WISDOM_CHESS_FAST_TESTS AND NOT WISDOM_CHESS_SLOW_TESTS)
    return()
endif()

find_program(WISDOM_CHESS_ADB adb
    HINTS
        "${ANDROID_SDK_ROOT}/platform-tools"
        "$ENV{ANDROID_SDK_ROOT}/platform-tools"
        "$ENV{ANDROID_HOME}/platform-tools"
    NO_CMAKE_FIND_ROOT_PATH
    DOC "adb, which runs the tests on the Android device")
find_library(WISDOM_CHESS_ANDROID_LIBCXX c++_shared
    DOC "The NDK's libc++_shared.so, which the tests need on the Android device")

foreach(required WISDOM_CHESS_ADB WISDOM_CHESS_ANDROID_LIBCXX)
    if(NOT ${required})
        message(FATAL_ERROR "-- wisdom-chess: ${required} was not found, and the tests of an Android build "
            "need it. Set it, or build without the tests: "
            "-DWISDOM_CHESS_FAST_TESTS=Off -DWISDOM_CHESS_SLOW_TESTS=Off")
    endif()
endforeach()

configure_file("${CMAKE_CURRENT_LIST_DIR}/android-run-test.sh.in"
    "${CMAKE_BINARY_DIR}/android-run-test.sh" @ONLY)
set(CMAKE_CROSSCOMPILING_EMULATOR "${CMAKE_BINARY_DIR}/android-run-test.sh")

message("-- wisdom-chess: Android tests run through ${WISDOM_CHESS_ADB}; "
    "connect a device or emulator before building")
