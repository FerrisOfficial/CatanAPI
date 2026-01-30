#pragma once

#include "../itPlayers/it5Player.hpp"

#include <array>

// ParaSettleIt5Player: It5 logic for everything except initial placement.
// Initial placement uses a parameterized heuristic (compatible with ParaPlayer's init/prod/policy keys).
struct ParaSettleIt5Player : public It5Player {
    ParaSettleIt5Player() : It5Player() {}
    virtual ~ParaSettleIt5Player() = default;

    std::pair<Action::PackedAction, Action::PackedAction> getInitialPlacement() override;
    std::pair<Action::PackedAction, Action::PackedAction> get2InitialPlacement() override;

private:
    std::array<bool, 5> firstPlacementResources {false, false, false, false, false};
};

// Backwards-compatible alias (older name, used by older scripts/flags).
using ParaSetIt5Player = ParaSettleIt5Player;
