#pragma once

#include "wisdom-chess/engine/global.hpp"

#include "wisdom-chess/engine/board_code.hpp"
#include "wisdom-chess/engine/coord.hpp"
#include "wisdom-chess/engine/coord_set.hpp"
#include "wisdom-chess/engine/material.hpp"
#include "wisdom-chess/engine/move.hpp"
#include "wisdom-chess/engine/piece.hpp"
#include "wisdom-chess/engine/position.hpp"

namespace wisdom
{
    class BoardBuilder;
    class BoardPieces;

    class Board
    {
    public:
        Board();

        Board (const Board& board) = default;

        auto operator= (const Board& board) -> Board& = default;

        explicit Board (const BoardBuilder& builder);

        // A FEN string that does not parse is a precondition failure. Text
        // from outside the program goes through FenParser::parse().
        [[nodiscard]] static auto fromFen (const string& fen) -> Board;

        // Whether the boards are the same position for repetition, as
        // getBoardCode() identifies it.
        [[nodiscard]] friend auto
        operator== (const Board& a, const Board& b) noexcept
            -> bool
        {
            if (a.getBoardCode() != b.getBoardCode())
                return false;

            return a.my_squares == b.my_squares;
        }

        [[nodiscard]] constexpr auto
        pieceAt (int row, int col) const noexcept
            -> optional<ColoredPiece>
        {
            Coord coord = Coord::make (row, col);
            return my_squares[coord.index()];
        }

        [[nodiscard]] constexpr auto
        pieceAt (Coord coord) const noexcept
            -> optional<ColoredPiece>
        {
            return my_squares[coord.index()];
        }

        friend auto
        operator<< (std::ostream& os, const Board& board)
            -> std::ostream&;

        // Writes the board to std::cerr. Meant to be called from a debugger.
        void dump() const;

        [[nodiscard]] auto
        getHalfMoveClock() const noexcept
            -> int
        {
            return my_half_move_clock;
        }

        [[nodiscard]] auto
        getFullMoveClock() const noexcept
            -> int
        {
            return my_full_move_clock;
        }

        // Convert the board to a string.
        [[nodiscard]] auto
        asString() const
            -> string;

        [[nodiscard]] auto
        getMaterial() const& noexcept
            -> const Material&
        {
            return my_material;
        }
        void getMaterial() const&& = delete;

        [[nodiscard]] auto
        getPosition() const& noexcept
            -> const Position&
        {
            return my_position;
        }
        void getPosition() const&& = delete;

        [[nodiscard]] auto
        toFenString (Color turn) const
            -> string;
        [[nodiscard]] auto
        castledString (Color color) const
            -> string;

        // Create a new board with the move applied:
        [[nodiscard]] auto
        withMove (Color who, Move move) const noexcept
            -> Board;

        // Create a new board with the current turn updated:
        [[nodiscard]] auto
        withCurrentTurn (Color who) const noexcept
            -> Board;

        // The squares of one side's pieces.
        [[nodiscard]] auto
        getPieceCoords (Color who) const noexcept
            -> CoordSet
        {
            return CoordSet { my_occupied[colorIndex (who)] };
        }

        // One side's pieces with their squares.
        [[nodiscard]] auto
        getPieces (Color who) const& noexcept
            -> BoardPieces;
        void getPieces (Color who) const&& = delete;

        [[nodiscard]] auto
        getKingPosition (Color who) const noexcept
            -> Coord
        {
            return my_king_pos[colorIndex (who)];
        }

        [[nodiscard]] auto
        getCastlingEligibility (Color who) const noexcept
            -> CastlingEligibility
        {
            return my_code.getCastleState (who);
        }

        [[nodiscard]] auto
        ableToCastle (Color who, CastlingEligibility castle_types) const noexcept
            -> bool
        {
            auto castle_state = getCastlingEligibility (who);
            auto castle_bits = to_uint<uint8_t> (castle_state);

            // Asking about no side at all is never satisfied: check for
            // every bit, which no state has.
            auto check_bits = castle_types == CastlingEligibility::Neither_Side
                ? std::numeric_limits<uint8_t>::max()
                : to_uint<uint8_t> (castle_types);

            bool has_rights = (castle_bits & check_bits) == check_bits;

            return has_rights;
        }

        [[nodiscard]] auto
        getCurrentTurn() const noexcept
            -> Color
        {
            return my_code.getCurrentTurn();
        }

        // The en passant target as FEN records it, whether or not a legal
        // capture of it exists.
        [[nodiscard]] auto
        getAnyEnPassantTarget() const noexcept
            -> optional<EnPassantTarget>
        {
            return my_code.getAnyEnPassantTarget();
        }

        // The en passant target, if the side to move has a legal capture
        // of it.
        [[nodiscard]] auto
        getLegalEnPassantTarget() const noexcept
            -> optional<EnPassantTarget>
        {
            return my_code.getLegalEnPassantTarget();
        }

        // FEN records the passed square after every double pawn push, but
        // for repetition FIDE only counts an en passant capture that is
        // actually possible (Laws of Chess 9.2.3.1). This code leaves the
        // en passant target out when no legal capture reaches it. Castling
        // rights stay in, since FIDE compares rights, not whether castling
        // is possible on this move (9.2.3.2).
        [[nodiscard]] auto
        getBoardCode() const noexcept
            -> BoardCode
        {
            return my_code.withoutIllegalEnPassantTarget();
        }

        // The code with the en passant target as FEN records it.
        [[nodiscard]] auto
        getUnnormalizedBoardCode() const noexcept
            -> BoardCode
        {
            return my_code;
        }

        [[nodiscard]] static constexpr auto
        allCoords() noexcept
            -> CoordIterator
        {
            return CoordIterator {};
        }

        [[nodiscard]] auto
        findFirstCoordWithPiece (
            Piece piece,
            Coord starting_at = First_Coord
        ) const noexcept
            -> optional<Coord>;

    private:
        void makeMove (Color who, Move move) noexcept;

        // Whether each stored king position holds that color's king.
        [[nodiscard]] auto
        hasKingsAtKingPositions() const noexcept
            -> bool;

        // Whether each side's occupied squares are those its pieces are on.
        [[nodiscard]] auto
        occupiedMatchesSquares() const noexcept
            -> bool;

        auto applyForEnPassant (Color who, Coord src, Coord dst) noexcept -> ColoredPiece;
        void updateEnPassantEligibility (Color who, ColoredPiece src_piece, Move move) noexcept;
        void classifyEnPassantTarget() noexcept;
        void clearEnPassantTarget() noexcept;

        void applyForCastlingMove (
            Move king_move,
            [[maybe_unused]] Coord src,
            [[maybe_unused]] Coord dst
        ) noexcept;
        void updateAfterKingMove (Color who, [[maybe_unused]] Coord src, Coord dst) noexcept;
        void setCastleState (Color who, CastlingEligibility new_state) noexcept;

        void removeCastlingEligibility (
            Color who,
            CastlingEligibility removed_castle_states
        ) noexcept;

        void updateAfterRookCapture (
            Color opponent,
            ColoredPiece dst_piece,
            Coord src,
            Coord dst
        ) noexcept;

        void updateAfterRookMove (
            Color player,
            [[maybe_unused]] ColoredPiece src_piece,
            Coord src
        ) noexcept;

        void setKingPosition (Color who, Coord pos) noexcept;
        void setPiece (Coord coord, optional<ColoredPiece> piece) noexcept;
        void updateMoveClock (Color who, Piece orig_src_piece_type, Move move) noexcept;
        void setCurrentTurn (Color who) noexcept;

    private:
        // The representation of the board. This must be declared first:
        // the Position and Material constructors read it through *this.
        array<optional<ColoredPiece>, Num_Squares> my_squares;

        // Keep track of hashing information.
        BoardCode my_code;

        // Number of half moves since pawn or capture.
        int my_half_move_clock = 0;

        // Number of full moves, updated after black moves.
        int my_full_move_clock = 1;

        // Keep track of the positions on the board.
        Position my_position;

        // Keep track of the material on the board.
        Material my_material;

        // positions of the kings.
        array<Coord, Num_Players> my_king_pos;

        // The squares each side's pieces are on, one bit for each index.
        array<uint64_t, Num_Players> my_occupied {};
    };

    // A piece and the square it is on, which reads the piece from the
    // board.
    class PlacedPiece
    {
    public:
        constexpr
        PlacedPiece (nonnull<const Board> board, Coord coord) noexcept
            : my_board { board }
            , my_coord { coord }
        {}

        [[nodiscard]] constexpr auto
        coord() const noexcept
            -> Coord
        {
            return my_coord;
        }

        [[nodiscard]] auto
        piece() const noexcept
            -> ColoredPiece;

        template <size_t Index>
        [[nodiscard]] auto
        get() const noexcept
        {
            if constexpr (Index == 0)
                return coord();
            else
                return piece();
        }

    private:
        nonnull<const Board> my_board;
        Coord my_coord;
    };

    // One side's pieces on a board, in square index order.
    class BoardPieces
    {
    public:
        class Iterator
        {
        public:
            using difference_type = int;
            using value_type = PlacedPiece;
            using reference = PlacedPiece;
            using iterator_category = std::forward_iterator_tag;

            constexpr
            Iterator() noexcept = default;

            constexpr
            Iterator (nonnull<const Board> board, CoordSet::Iterator coords) noexcept
                : my_board { board }
                , my_coords { coords }
            {}

            [[nodiscard]] auto
            operator*() const noexcept
                -> PlacedPiece
            {
                return PlacedPiece { *my_board, *my_coords };
            }

            constexpr auto
            operator++() noexcept
                -> Iterator&
            {
                ++my_coords;
                return *this;
            }

            constexpr auto
            operator++ (int) noexcept
                -> Iterator
            {
                auto previous = *this;
                ++my_coords;
                return previous;
            }

            [[nodiscard]] constexpr auto
            operator== (const Iterator& other) const noexcept
                -> bool
            {
                return my_coords == other.my_coords;
            }

            [[nodiscard]] constexpr auto
            operator== (std::default_sentinel_t) const noexcept
                -> bool
            {
                return my_coords == CoordSet::Iterator {};
            }

        private:
            optional<nonnull<const Board>> my_board;
            CoordSet::Iterator my_coords;
        };

        constexpr
        BoardPieces (nonnull<const Board> board, CoordSet coords) noexcept
            : my_board { board }
            , my_coords { coords }
        {}

        [[nodiscard]] constexpr auto
        begin() const noexcept
            -> Iterator
        {
            return Iterator { my_board, my_coords.begin() };
        }

        [[nodiscard]] constexpr auto
        end() const noexcept  // NOLINT(readability-convert-member-functions-to-static)
            -> std::default_sentinel_t
        {
            return std::default_sentinel;
        }

    private:
        nonnull<const Board> my_board;
        CoordSet my_coords;
    };

    static_assert (std::forward_iterator<BoardPieces::Iterator>);
    static_assert (std::sentinel_for<std::default_sentinel_t, BoardPieces::Iterator>);

    inline auto
    PlacedPiece::piece() const noexcept
        -> ColoredPiece
    {
        auto piece = my_board->pieceAt (my_coord);
        ASSERT( piece.has_value() );
        return *piece;
    }

    inline auto
    Board::getPieces (Color who) const& noexcept
        -> BoardPieces
    {
        return BoardPieces { this, getPieceCoords (who) };
    }
}

template <>
struct std::tuple_size<wisdom::PlacedPiece>
    : std::integral_constant<size_t, 2>
{
};

template <>
struct std::tuple_element<0, wisdom::PlacedPiece>
{
    using type = wisdom::Coord;
};

template <>
struct std::tuple_element<1, wisdom::PlacedPiece>
{
    using type = wisdom::ColoredPiece;
};
