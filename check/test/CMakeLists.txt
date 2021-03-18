#
# Tests
#

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake.modules/")

# Make test executable
add_executable(chess_tests
        test_main.cpp
        piece_test.cpp
        coord_test.cpp
        en_passant_test.cpp
        material_test.cpp
        castle_test.cpp
        pawn_promotion_test.cpp
        board_builder_test.cpp
        check_test.cpp
        position_test.cpp
        move_test.cpp
        search_test.cpp
        parse_simple_move_test.cpp
        fen_test.cpp
        move_list_test.cpp
        board_code_test.cpp
        history_test.cpp
        $<TARGET_OBJECTS:chess_lib>
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  # using Clang
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # using GCC
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O3 -march=native -Wall -Wno-unused-parameter -Wno-unused-function")
  #set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -march=native")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native")
  #set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native")

  # Speed up linking on Linux
  execute_process(COMMAND ${CMAKE_C_COMPILER} -fuse-ld=gold -Wl,--version ERROR_QUIET OUTPUT_VARIABLE ld_version)
  if ("${ld_version}" MATCHES "GNU gold")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=gold -Wl,--disable-new-dtags -lpthread")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fuse-ld=gold -Wl,--disable-new-dtags -lpthread")
  endif()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
  # using Intel C++
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  # using Visual Studio C++
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /std:c++latest")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 /std:c++latest")
endif()
