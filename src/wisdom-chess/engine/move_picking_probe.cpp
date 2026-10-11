#include "wisdom-chess/engine/move_picking_probe.hpp"

namespace wisdom
{
    MovePickingProbe move_picking_probe; // NOLINT(*-avoid-non-const-global-variables)

    void MovePickingProbe::Counts::recordCutoff (size_t index, Stage stage) noexcept
    {
        cutoffs++;
        cutoff_index_sum += narrow<int64_t> (index + 1);

        size_t bucket = index < 3 ? index : index < 8 ? 3 : 4;
        cutoff_index[bucket]++;
        cutoff_stage[stage]++;
    }
}
