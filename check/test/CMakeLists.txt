#
# Tests
#

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake.modules/")

add_executable(chess_tests
        ../src/board.cpp ../src/check.cpp ../src/evaluate.cpp ../src/generate.cpp
        ../src/material.cpp ../src/move.cpp ../src/move_tree.cpp ../src/piece.cpp ../src/search.cpp
        ../src/sha1.cpp ../src/str.cpp ../src/move_timer.cpp ../src/global.h
        ../src/validate.cpp ../src/position.cpp ../src/board_builder.cpp ../src/game.cpp ../src/game.h
        ../src/board_builder.hpp

        test_main.cpp
        piece_test.cpp
        coord_test.cpp
        en_passant_test.cpp
        material_test.cpp
        castle_test.cpp
        pawn_promotion_test.cpp
        board_builder_test.cpp
        check_test.cpp position_test.cpp move_test.cpp search_test.cpp parse_simple_move.cpp parse_simple_move.hpp
        parse_simple_move_test.cpp ../src/coord.cpp
        ../src/fen.cpp ../src/fen.hpp fen_test.cpp ../src/move_list.cpp ../src/move_list.hpp move_list_test.cpp ../src/move_history.hpp ../src/move_history.cpp ../src/multithread_search.cpp ../src/multithread_search.h ../src/coord_iterator.cpp ../src/coord_iterator.hpp ../src/board_code.cpp ../src/board_code.hpp board_code_test.cpp ../src/output.cpp ../src/output.hpp ../src/board_history.cpp ../src/board_history.hpp ../src/history.cpp ../src/history.hpp)

		if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  # using Clang
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # using GCC
  #set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -fsanitize=address")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -march=native")
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


if (HAVE_WINSOCK2_H)
    TARGET_LINK_LIBRARIES(chess_tests ws2_32)
endif (HAVE_WINSOCK2_H)

INCLUDE(ParseAndAddCatchTests)
ParseAndAddCatchTests(chess_tests)