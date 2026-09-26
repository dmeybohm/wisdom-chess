add_executable(wisdom-chess-console main.cpp play.cpp)

# wisdom::viewmodel links wisdom::chess publicly. Naming it again here makes
# the Apple linker warn about a duplicate library.
target_link_libraries(wisdom-chess-console PRIVATE wisdom::viewmodel)
wisdom_chess_enable_warnings(wisdom-chess-console)

if (WISDOM_CHESS_PCH_ENABLED)
    target_precompile_headers(wisdom-chess-console PRIVATE ../../engine/global.hpp)
endif()

install(TARGETS wisdom-chess-console
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")

# Every script makes Black human first, so that no search runs and the
# output does not depend on the engine's choice of move.
if (WISDOM_CHESS_FAST_TESTS)
    set(console_save_file "${CMAKE_CURRENT_BINARY_DIR}/console-save-test.txt")

    wisdom_chess_add_cli_test("Console: help" wisdom-chess-console
        INPUT "help" "quit"
        EXPECTED "Available commands:" "o-o-o +Castle queenside")

    wisdom_chess_add_cli_test("Console: input can end without quit" wisdom-chess-console
        INPUT "human_black"
        EXPECTED "Black player set to human")

    wisdom_chess_add_cli_test("Console: available moves" wisdom-chess-console
        INPUT "human_black" "moves" "quit"
        EXPECTED "Available moves:" "\\[e2 e4\\]" "\\[g1 f3\\]")

    wisdom_chess_add_cli_test("Console: an invalid command is reported" wisdom-chess-console
        INPUT "human_black" "xyzzy" "e2e5" "quit"
        EXPECTED "Invalid move or command.*Invalid move or command")

    wisdom_chess_add_cli_test("Console: checkmate" wisdom-chess-console
        INPUT "human_black" "f2f3" "e7e5" "g2g4" "d8h4" "quit"
        EXPECTED "\\(Black\\)\\? " "Checkmate - Black wins the game\\.")

    wisdom_chess_add_cli_test("Console: check" wisdom-chess-console
        INPUT "human_black" "e2e4" "f7f6" "d1h5" "quit"
        EXPECTED "Check!"
        FORBIDDEN "Checkmate")

    wisdom_chess_add_cli_test("Console: a position loaded from a FEN string" wisdom-chess-console
        INPUT "fen" "7k/5Q2/6K1/8/8/8/8/8 b - - 0 1" "quit"
        EXPECTED "New game successfully loaded" "Stalemate - No legal moves for Black")

    wisdom_chess_add_cli_test("Console: an invalid FEN string" wisdom-chess-console
        INPUT "human_black" "fen" "not a fen" "quit"
        EXPECTED "Failed loading FEN string")

    wisdom_chess_add_cli_test("Console: number prompts reject what is not a number in range" wisdom-chess-console
        INPUT "human_black" "maxdepth" "99999999999999999999" "timeout" "abc" "maxdepth" "4" "timeout" "3" "quit"
        EXPECTED
            "Invalid search depth\\."
            "Invalid search timeout\\."
            "Max depth set to 4\\."
            "Timeout set to 3 seconds\\.")

    wisdom_chess_add_cli_test("Console: zero is not a search depth or timeout" wisdom-chess-console
        INPUT "human_black" "maxdepth" "0" "timeout" "0" "maxdepth" "-1" "quit"
        EXPECTED
            "Invalid search depth\\..*Invalid search timeout\\..*Invalid search depth\\."
        FORBIDDEN "Max depth set to" "Timeout set to")

    wisdom_chess_add_cli_test("Console: an empty answer to a draw offer asks again" wisdom-chess-console
        INPUT "human_black"
            "g1f3" "g8f6" "f3g1" "f6g8" "g1f3" "g8f6" "f3g1" "f6g8"
            "" "maybe" "y" "quit"
        EXPECTED
            "Would you like a draw\\?.*Would you like a draw\\?.*Would you like a draw\\?"
            "Draw - Threefold repetition rule\\.")

    wisdom_chess_add_cli_test("Console: a capital Y accepts a draw by repetition" wisdom-chess-console
        INPUT "human_black"
            "g1f3" "g8f6" "f3g1" "f6g8" "g1f3" "g8f6" "f3g1" "f6g8"
            "Y" "quit"
        EXPECTED
            "Threefold repetition detected\\. Would you like a draw\\?"
            "Draw - Threefold repetition rule\\.")

    wisdom_chess_add_cli_test("Console: a declined draw by repetition" wisdom-chess-console
        INPUT "human_black"
            "g1f3" "g8f6" "f3g1" "f6g8" "g1f3" "g8f6" "f3g1" "f6g8"
            "n" "quit"
        EXPECTED "Threefold repetition detected\\. Would you like a draw\\?"
        FORBIDDEN "Draw - ")

    wisdom_chess_add_cli_test("Console: a saved game loads back" wisdom-chess-console
        INPUT "human_black" "e2e4" "e7e5" "save" "${console_save_file}"
            "load" "${console_save_file}" "moves" "quit"
        EXPECTED
            "Game saved to "
            "New game successfully loaded.*\\[g1 f3\\]"
        FORBIDDEN "\\[e2 e4\\]")

    wisdom_chess_add_cli_test("Console: a save that cannot be written is reported" wisdom-chess-console
        INPUT "human_black" "e2e4" "save" "${CMAKE_CURRENT_BINARY_DIR}/no-such-dir/game.txt" "quit"
        EXPECTED "Error saving game: Cannot open .*no-such-dir/game.txt for writing\\."
        FORBIDDEN "Game saved to")

    wisdom_chess_add_cli_test("Console: a missing saved game" wisdom-chess-console
        INPUT "human_black" "load" "${CMAKE_CURRENT_BINARY_DIR}/no-such-file.txt" "quit"
        EXPECTED "Error loading game")

    wisdom_chess_add_cli_test("Console: players and pausing" wisdom-chess-console
        INPUT "pause" "computer_white" "human_white" "human_black" "unpause" "switch" "quit"
        EXPECTED
            "Game engine paused"
            "White player set to computer"
            "White player set to human"
            "Black player set to human"
            "Game engine unpaused"
            "Players switched.*\\(Black\\)\\? ")
endif()
