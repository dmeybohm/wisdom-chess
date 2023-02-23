#ifndef WISDOM_CHESS_BOARD_BUILDER_HPP
#define WISDOM_CHESS_BOARD_BUILDER_HPP

#include "global.hpp"
#include "piece.hpp"
#include "move.hpp"

namespace wisdom
{
    class BoardBuilderError : public Error
    {
    public:
        explicit BoardBuilderError (const string &message) :
                Error (message)
        {}
    };

    class BoardBuilder final
    {
    public:
        using PieceRow = array<Piece, Num_Columns>;

        struct CoordAndPiece
        {
            czstring coord;
            Piece piece_type;
        };

        constexpr BoardBuilder () : my_squares { empty_squares () }
        {}

        static auto from_default_position () -> BoardBuilder;

        [[nodiscard]] static constexpr auto empty_squares () -> array<ColoredPiece, Num_Squares>
        {
            array<ColoredPiece, Num_Squares> result {};
            for (auto i = 0; i < Num_Squares; i++)
                result[i] = Piece_And_Color_None;
            return result;
        }

        static constexpr PieceRow Default_Piece_Row = {
            Piece::Rook, Piece::Knight, Piece::Bishop, Piece::Queen, Piece::King,
            Piece::Bishop, Piece::Knight, Piece::Rook,
        };

        void add_piece (const string& coord_str, Color who, Piece piece_type);

        void add_piece (int row, int col, Color who, Piece piece_type);

        void add_pieces (Color who, const vector<CoordAndPiece>& pieces);

        void add_row_of_same_color (int row, Color who, const PieceRow& piece_types);

        void add_row_of_same_color (const string& coord_str, Color who, const PieceRow& piece_types);

        void add_row_of_same_color_and_piece (int row, Color who, Piece piece_type);

        void add_row_of_same_color_and_piece (const string& coord_str, Color who, Piece piece_type);

        void set_en_passant_target (Color vulnerable_color, const string& coord_str);

        void set_castling (Color who, CastlingEligibility state);

        void set_current_turn (Color who);

        void set_half_moves_clock (int new_half_moves_clock);

        void set_full_moves (int new_full_moves);

        [[nodiscard]] auto get_squares () const& -> const array<ColoredPiece, Num_Squares>&
        {
            return my_squares;
        }
        void get_squares () const&& = delete;

        [[nodiscard]] auto piece_at (Coord coord) const -> ColoredPiece
        {
            assert (coord_index (coord) < Num_Squares);
            return my_squares[coord_index (coord)];
        }

        [[nodiscard]] auto get_current_turn () const -> Color
        {
            return my_current_turn;
        }

        [[nodiscard]] auto get_en_passant_targets () const -> array<Coord, Num_Players>
        {
            return my_en_passant_targets;
        }

        [[nodiscard]] auto get_castle_state (Color who) const -> CastlingEligibility
        {
            if (my_castle_states[color_index (who)])
                return *my_castle_states[color_index (who)];
            return calculate_castle_state_from_position (who);
        }

        [[nodiscard]] auto get_king_position (Color who) -> Coord
        {
            auto index = color_index (who);
            if (!my_king_positions[index])
                throw BoardBuilderError { "Missing king position in constructing board." };
            return *my_king_positions[index];
        }

        [[nodiscard]] auto get_king_positions () const -> array<Coord, Num_Players>
        {
            if (!my_king_positions[Color_Index_White] || !my_king_positions[Color_Index_Black])
                throw BoardBuilderError { "Missing king position in constructing board." };
            return { *my_king_positions[Color_Index_White], *my_king_positions[Color_Index_Black] };
        }

        [[nodiscard]] auto get_half_moves_clock () const -> int
        {
            return my_half_moves_clock;
        }

        [[nodiscard]] auto get_full_moves () const -> int
        {
            return my_full_moves;
        }

    private:
        [[nodiscard]] auto calculate_castle_state_from_position (Color who) const -> CastlingEligibility;

        array<ColoredPiece, Num_Squares> my_squares;
        array<Coord, Num_Players> my_en_passant_targets {
            No_En_Passant_Coord, No_En_Passant_Coord
        };

        array<optional<CastlingEligibility>, Num_Players> my_castle_states {
            nullopt, nullopt
        };

        Color my_current_turn = Color::White;
        int my_half_moves_clock = 0;
        int my_full_moves = 0;
        array<optional<Coord>, Num_Players> my_king_positions {
            nullopt, nullopt
        };
    };

}

#endif //WISDOM_CHESS_BOARD_BUILDER_HPP
