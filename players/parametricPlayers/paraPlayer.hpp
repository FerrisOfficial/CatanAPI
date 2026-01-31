#pragma once

#include "game_simulation/actions.hpp"
#include "players/player.hpp"

class ParaPlayer : public IPlayer {
   public:
    ParaPlayer();
    virtual ~ParaPlayer() = default;

    std::pair<Action::PackedAction, Action::PackedAction> getInitialPlacement()
        override;
    std::pair<Action::PackedAction, Action::PackedAction> get2InitialPlacement()
        override;

    Action::PackedAction getDevAction() override;
    Action::PackedAction getDiscardAction() override;
    Action::PackedAction getMoveRobber() override;
    Action::PackedAction getTurnAction() override;
};
