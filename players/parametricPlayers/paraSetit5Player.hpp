#pragma once

#include <array>

#include "itPlayers/it5Player.hpp"

struct ParaSettleIt5Player : public It5Player {
    ParaSettleIt5Player() : It5Player() {}
    virtual ~ParaSettleIt5Player() = default;

    std::pair<Action::PackedAction, Action::PackedAction> getInitialPlacement() override;
    std::pair<Action::PackedAction, Action::PackedAction> get2InitialPlacement() override;

private:
    std::array<bool, 5> firstPlacementResources {false, false, false, false, false};
};

using ParaSetIt5Player = ParaSettleIt5Player;
