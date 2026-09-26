cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(cmake/CPM.cmake)
CPMUsePackageLock(cpm-package-lock.cmake)
include(cmake/Warnings.cmake)
include(cmake/CliTests.cmake)
include(CheckCXXSourceCompiles)

project(WisdomChess
    VERSION 0.1.0
    DESCRIPTION "Wisdom Chess"
    HOMEPAGE_URL "https://github.com/dmeybohm/wisdom-chess"
    LANGUAGES CXX)

enable_testing()

# Auto-detect FIL-C compiler
check_cxx_source_compiles("
    int main() {
    #ifndef __PIZLONATOR_WAS_HERE__
        #error Not FIL-C
    #endif
        return 0;
    }
" WISDOM_CHESS_FILC_DETECTED)

if(NOT DEFINED WISDOM_CHESS_FILC_DETECTED)
    set(WISDOM_CHESS_FILC_DETECTED OFF)
endif()

# The React build is integrated by default only for WebAssembly builds.
if(EMSCRIPTEN)
    set(WISDOM_CHESS_REACT_BUILD_INTEGRATED_DEFAULT ON)
else()
    set(WISDOM_CHESS_REACT_BUILD_INTEGRATED_DEFAULT OFF)
endif()

option(WISDOM_CHESS_WERROR "Treat compiler warnings as errors (on in CI)" Off)
option(WISDOM_CHESS_ASAN "Enable AddressSanitizer and UndefinedBehaviorSanitizer")
option(WISDOM_CHESS_TSAN "Enable ThreadSanitizer")
option(WISDOM_CHESS_FAST_TESTS "Enable building of fast tests" On)
option(WISDOM_CHESS_SLOW_TESTS "Enable building of slow tests" Off)
option(WISDOM_CHESS_CONSOLE_UI "Enable building of console chess game" On)
option(WISDOM_CHESS_REACT_UI "Enable building of React UI" On)
option(WISDOM_CHESS_REACT_BUILD_INTEGRATED "Build React frontend as part of CMake build" ${WISDOM_CHESS_REACT_BUILD_INTEGRATED_DEFAULT})
option(WISDOM_CHESS_PCH_ENABLED "Enable Pre-compiled headers" On)
option(WISDOM_CHESS_TOOLS "Build research/analysis tools" Off)
option(WISDOM_CHESS_BENCHMARKS "Enable building of benchmarks" Off)
option(WISDOM_CHESS_FILC_COMPAT "Enable FIL-C runtime compatibility (auto-detected, disables POSIX signals in doctest)" ${WISDOM_CHESS_FILC_DETECTED})
option(WISDOM_CHESS_INSTALLER "Build a Qt Installer Framework installer for the desktop QML app (target: installer)" Off)

# QML UI option: AUTO (build if Qt6 found), ON (require Qt6), OFF (disable)
set(WISDOM_CHESS_QML_UI "AUTO" CACHE STRING "Build QML UI: AUTO (if Qt6 found), ON (require Qt6), OFF (disable)")
set_property(CACHE WISDOM_CHESS_QML_UI PROPERTY STRINGS AUTO ON OFF)
set(WISDOM_CHESS_QT_DIR "" CACHE PATH "Directory for Qt libraries")

# Enable -O3 in RelWithDebugInfo
if(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        message("-- wisdom-chess: Re-enabling -O3 option for G++/Clang")
        string(REPLACE "-O2" "" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}") 
        set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -O3") 
    endif()
endif()

if (WISDOM_CHESS_ASAN AND WISDOM_CHESS_TSAN)
    message(FATAL_ERROR "WISDOM_CHESS_ASAN and WISDOM_CHESS_TSAN cannot be enabled together")
endif()

if (WISDOM_CHESS_ASAN)
    message("-- wisdom-chess: Enabling AddressSanitizer and UndefinedBehaviorSanitizer")
    if (MSVC)
        add_compile_options(/fsanitize=address)
        add_link_options(/fsanitize=address /zi)
    else()
        add_compile_options(-fsanitize=address,undefined -fno-sanitize-recover=undefined -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address,undefined)
    endif()
endif()

if (WISDOM_CHESS_TSAN)
    if (MSVC)
        message(FATAL_ERROR "WISDOM_CHESS_TSAN is not supported with MSVC")
    endif()
    message("-- wisdom-chess: Enabling ThreadSanitizer")
    add_compile_options(-fsanitize=thread -fno-omit-frame-pointer)
    add_link_options(-fsanitize=thread)
endif()

if (WISDOM_CHESS_FILC_COMPAT)
    message("-- wisdom-chess: Enabling FIL-c runtime compatibility")
endif()

if(WISDOM_CHESS_INSTALLER AND WIN32)
    # The Windows installer bundles and runs the MSVC redistributable, so
    # resolve it here where src/wisdom-chess/ui/qml can pick it up.
    if(NOT MSVC)
        message(FATAL_ERROR "-- wisdom-chess: the Windows installer build requires MSVC (it bundles vc_redist.x64.exe)")
    endif()
    # VCToolsRedistDir comes from vcvars with backslashes and a trailing
    # separator; normalise it so the path survives being written into the
    # generated install scripts.
    file(TO_CMAKE_PATH "$ENV{VCToolsRedistDir}" _wisdom_vc_redist_dir)
    string(REGEX REPLACE "/+$" "" _wisdom_vc_redist_dir "${_wisdom_vc_redist_dir}")
    set(WISDOM_CHESS_VCREDIST "${_wisdom_vc_redist_dir}/vc_redist.x64.exe"
        CACHE FILEPATH "vc_redist.x64.exe to bundle in the Windows installer")
    if(NOT EXISTS "${WISDOM_CHESS_VCREDIST}")
        message(FATAL_ERROR "-- wisdom-chess: vc_redist.x64.exe not found at '${WISDOM_CHESS_VCREDIST}'. "
            "Configure from a Visual Studio developer prompt or pass -DWISDOM_CHESS_VCREDIST=<path>")
    endif()
endif()

add_subdirectory(src)

# The Python helpers under scripts/ are tested when an interpreter is found.
if(WISDOM_CHESS_FAST_TESTS AND NOT EMSCRIPTEN)
    find_package(Python3 COMPONENTS Interpreter)
    if(Python3_Interpreter_FOUND)
        add_test(NAME "Scripts: engine match tally"
            COMMAND Python3::Interpreter ${CMAKE_SOURCE_DIR}/scripts/tests/engine_match_tally_test.py)
        set_tests_properties("Scripts: engine match tally" PROPERTIES LABELS fast)
    endif()
endif()

if(WISDOM_CHESS_INSTALLER)
    if(EMSCRIPTEN OR ANDROID OR NOT TARGET WisdomChessQml)
        message(FATAL_ERROR "-- wisdom-chess: WISDOM_CHESS_INSTALLER needs the desktop QML UI "
            "(WISDOM_CHESS_QML_UI=ON with Qt found; not Emscripten/Android)")
    endif()
    include(cmake/Installer.cmake)
endif()

# C++ style linter (only for native builds, not WASM)
if(NOT EMSCRIPTEN)
    option(WISDOM_CHESS_BUILD_LINTER "Build the C++ style linter" ON)
    if(WISDOM_CHESS_BUILD_LINTER)
        add_subdirectory(scripts/linter)

        add_custom_target(lint
            COMMAND wisdom-linter ${CMAKE_SOURCE_DIR}/src
                ${CMAKE_SOURCE_DIR}/scripts/linter/main.cpp
                ${CMAKE_SOURCE_DIR}/scripts/linter/linter.cpp
                ${CMAKE_SOURCE_DIR}/scripts/linter/linter.hpp
                ${CMAKE_SOURCE_DIR}/scripts/linter/rules
            DEPENDS wisdom-linter
            COMMENT "Running C++ style linter"
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        )
    endif()
endif()
