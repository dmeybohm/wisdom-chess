#pragma once

#include "wisdom-chess/engine/global.hpp"

namespace wisdom
{
    // Measurement only, to be reverted: where in the ordered move list the
    // search finds its cutoffs. Not thread-safe; the search report is the
    // only reader.
    struct MovePickingProbe
    {
        // The stage a staged move picker would try a move in.
        enum Stage : size_t
        {
            Table,
            Capture,
            Promotion,
            Killer,
            Quiet,
            Num_Stages,
        };

        static constexpr size_t Num_Index_Buckets = 5;

        struct Counts
        {
            int64_t nodes = 0;
            int64_t moves_generated = 0;
            int64_t tactical_generated = 0;
            int64_t moves_tried = 0;
            int64_t cutoffs = 0;
            int64_t cutoff_index_sum = 0;

            // The cutoff move's position in the list: 1, 2, 3, 4 to 8, 9 or later.
            array<int64_t, Num_Index_Buckets> cutoff_index {};
            array<int64_t, Num_Stages> cutoff_stage {};

            // Quiet moves tried before a quiet move cut off.
            int64_t quiet_tried_at_quiet_cutoff = 0;
            int64_t quiet_generated_at_quiet_cutoff = 0;

            int64_t table_move_given = 0;
            int64_t table_move_in_list = 0;
            int64_t killers_given = 0;
            int64_t killers_in_list = 0;

            void recordCutoff (size_t index, Stage stage) noexcept;
        };

        Counts search;
        Counts quiescence;
    };

    extern MovePickingProbe move_picking_probe; // NOLINT(*-avoid-non-const-global-variables)
}
