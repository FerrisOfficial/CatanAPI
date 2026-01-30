#pragma once

#include "game_simulation/consts.hpp"
#include "itPlayers/it5Player.hpp"

struct OneResourcePlayer : public It5Player {
    OneResourcePlayer() : It5Player() {}
    virtual ~OneResourcePlayer() = default;

    std::pair<Action::PackedAction, Action::PackedAction> getInitialPlacement() override;
    std::pair<Action::PackedAction, Action::PackedAction> get2InitialPlacement() override;
    Action::PackedAction getDiscardAction() override;
    Action::PackedAction getTurnAction() override;


private:
    NodeId firstSettlementNode = 0xFF;
    std::array<bool, 5> firstPlacementResources {false, false, false, false, false};
    Resource prioritizedResource = Resource::Brick; // chosen at setup based on board
    bool priorityChosen = false;
};