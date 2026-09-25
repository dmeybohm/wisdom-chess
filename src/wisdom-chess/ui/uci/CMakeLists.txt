if (WISDOM_CHESS_CONSOLE_UI)
    add_executable(wisdom-chess-uci
        main.cpp
        uci_interface.cpp
    )

    target_link_libraries(wisdom-chess-uci
        wisdom-chess-core
    )
    wisdom_chess_enable_warnings(wisdom-chess-uci)

    if (WISDOM_CHESS_PCH_ENABLED)
        target_precompile_headers(wisdom-chess-uci PRIVATE ../../engine/global.hpp)
    endif()
endif()

# Each script ends with "stop" before "quit". Both "quit" and the end of input
# supersede a running search, which then prints nothing. After "stop" the
# search owes its "bestmove" and "quit" waits for it.
if (WISDOM_CHESS_CONSOLE_UI AND WISDOM_CHESS_FAST_TESTS)
    wisdom_chess_add_cli_test("UCI: handshake" wisdom-chess-uci
        INPUT "uci" "isready" "quit"
        EXPECTED
            "id name Wisdom Chess"
            "option name Hash type spin default 16 min 1 max 1024"
            "option name Depth type spin"
            "uciok.*readyok"
        FORBIDDEN "bestmove")

    wisdom_chess_add_cli_test("UCI: unknown commands and blank lines are ignored" wisdom-chess-uci
        INPUT "" "xyzzy 1 2 3" "   " "isready" "quit"
        EXPECTED "^readyok")

    wisdom_chess_add_cli_test("UCI: input can end without quit" wisdom-chess-uci
        INPUT "isready"
        EXPECTED "readyok")

    wisdom_chess_add_cli_test("UCI: finds a mate in one" wisdom-chess-uci
        INPUT "position fen 6k1/5ppp/8/8/8/8/8/R3K3 w - - 0 1" "go depth 2" "stop" "quit"
        EXPECTED "bestmove a1a8")

    wisdom_chess_add_cli_test("UCI: the moves after startpos are played" wisdom-chess-uci
        INPUT "position startpos moves f2f3 e7e5 g2g4" "go depth 2" "stop" "quit"
        EXPECTED "bestmove d8h4")

    wisdom_chess_add_cli_test("UCI: no move in a checkmated position" wisdom-chess-uci
        INPUT "position startpos moves f2f3 e7e5 g2g4 d8h4" "go depth 2" "stop" "quit"
        EXPECTED "bestmove \\(none\\)")

    # The black king would win the pawn if White waited, so White promotes
    # now. With the king out of reach a shallow search may wait, since
    # quiescence still sees the promotion.
    wisdom_chess_add_cli_test("UCI: a promotion is written in lower case" wisdom-chess-uci
        INPUT "position fen 8/P1k5/8/8/8/8/8/K7 w - - 0 1" "go depth 2" "stop" "quit"
        EXPECTED "bestmove a7a8q")

    # Promoting to a rook is checkmate here. Promoting to a knight leaves
    # Black one legal move.
    wisdom_chess_add_cli_test("UCI: a rook promotion in the move list" wisdom-chess-uci
        INPUT "position fen 7k/P7/6K1/8/8/8/8/8 w - - 0 1 moves a7a8r" "go depth 2" "stop" "quit"
        EXPECTED "bestmove \\(none\\)")

    wisdom_chess_add_cli_test("UCI: a knight promotion in the move list" wisdom-chess-uci
        INPUT "position fen 7k/P7/6K1/8/8/8/8/8 w - - 0 1 moves a7a8n" "go depth 2" "stop" "quit"
        EXPECTED "bestmove h8g8")

    # The mating rook move starts from the square castling put the rook on.
    wisdom_chess_add_cli_test("UCI: kingside castling in the move list" wisdom-chess-uci
        INPUT "position fen 7k/p5pp/8/8/8/8/8/4K2R w K - 0 1 moves e1g1 a7a6" "go depth 2" "stop" "quit"
        EXPECTED "bestmove f1f8")

    wisdom_chess_add_cli_test("UCI: queenside castling in the move list" wisdom-chess-uci
        INPUT "position fen 7k/p5pp/8/8/8/8/8/R3K3 w Q - 0 1 moves e1c1 a7a6" "go depth 2" "stop" "quit"
        EXPECTED "bestmove d1d8")

    wisdom_chess_add_cli_test("UCI: stop answers an infinite search with a move" wisdom-chess-uci
        INPUT "position startpos" "go infinite" "stop" "quit"
        EXPECTED "bestmove [a-h][1-8][a-h][1-8]"
        FORBIDDEN "bestmove.*bestmove")

    wisdom_chess_add_cli_test("UCI: a superseded search stays silent" wisdom-chess-uci
        INPUT "position startpos" "go infinite" "position startpos moves e2e4" "go depth 1" "stop" "quit"
        EXPECTED "bestmove [a-h][1-8][a-h][1-8]"
        FORBIDDEN "bestmove.*bestmove")

    wisdom_chess_add_cli_test("UCI: the Depth option limits the search" wisdom-chess-uci
        INPUT "setoption name Depth value 1" "position startpos" "go" "stop" "quit"
        EXPECTED "info Searching depth 1" "bestmove [a-h][1-8][a-h][1-8]"
        FORBIDDEN "Searching depth 2")

    # The transposition table belongs to the interface, not to the game, so it
    # survives the "position" command and the per-"go" copy of the game.
    wisdom_chess_add_cli_test("UCI: consecutive positions each answer" wisdom-chess-uci
        INPUT "position startpos" "go depth 2" "stop"
              "position startpos moves e2e4 e7e5" "go depth 2" "stop" "quit"
        EXPECTED "bestmove [a-h][1-8][a-h][1-8].*bestmove [a-h][1-8][a-h][1-8]")

    # The second search probes a table the first one filled on this position.
    wisdom_chess_add_cli_test("UCI: a warm table still finds a mate in one" wisdom-chess-uci
        INPUT "position fen 6k1/5ppp/8/8/8/8/8/R3K3 w - - 0 1" "go depth 2" "stop"
              "position fen 6k1/5ppp/8/8/8/8/8/R3K3 w - - 0 1" "go depth 2" "stop" "quit"
        EXPECTED "bestmove a1a8.*bestmove a1a8")

    # A repetition draw is a property of the path, not of the board, so the
    # scores the first line produces must not survive into an unrelated
    # position. The fresh starting position is worth 90 here, not the 0 that
    # the repetition line stored under the same board hashes.
    wisdom_chess_add_cli_test("UCI: an unrelated position does not inherit draw scores" wisdom-chess-uci
        INPUT "position startpos moves g1f3 g8f6 f3g1 f6g8" "go depth 4" "stop"
              "position startpos" "go depth 3" "stop" "quit"
        EXPECTED "score: 90 \\][ \t\r\n]*bestmove b1c3")

    wisdom_chess_add_cli_test("UCI: a search answers after the Hash option" wisdom-chess-uci
        INPUT "setoption name Hash value 1" "position startpos" "go depth 2" "stop" "quit"
        EXPECTED "bestmove [a-h][1-8][a-h][1-8]")

    # Resizing discards the warm table; the search that follows still answers.
    wisdom_chess_add_cli_test("UCI: the Hash option resizes between searches" wisdom-chess-uci
        INPUT "position startpos" "go depth 2" "stop" "setoption name Hash value 1"
              "position startpos moves d2d4" "go depth 2" "stop" "quit"
        EXPECTED "bestmove [a-h][1-8][a-h][1-8].*bestmove [a-h][1-8][a-h][1-8]")

    wisdom_chess_add_cli_test("UCI: ucinewgame between searches" wisdom-chess-uci
        INPUT "position startpos" "go depth 2" "stop" "ucinewgame"
              "position startpos moves d2d4" "go depth 2" "stop" "quit"
        EXPECTED "bestmove [a-h][1-8][a-h][1-8].*bestmove [a-h][1-8][a-h][1-8]")

    wisdom_chess_add_cli_test("UCI: an invalid FEN is reported in debug mode" wisdom-chess-uci
        INPUT "debug on" "position fen not a fen at all ok" "isready" "quit"
        EXPECTED "info string Invalid FEN: not a fen at all ok" "readyok")

    wisdom_chess_add_cli_test("UCI: an invalid FEN is ignored otherwise" wisdom-chess-uci
        INPUT "position fen not a fen at all ok" "isready" "quit"
        EXPECTED "readyok"
        FORBIDDEN "Invalid FEN")
endif()
