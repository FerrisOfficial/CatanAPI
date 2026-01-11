#pragma once

#include "it5Player.hpp"

// It6: It5 + stronger initial placements + improved robber + multi-action planning (MCTS-style) on deterministic actions.
struct It6Player : public It5Player {
    It6Player() : It5Player() {}
    virtual ~It6Player() = default;

    std::pair<Action::PackedAction, Action::PackedAction> getInitialPlacement() override;
    std::pair<Action::PackedAction, Action::PackedAction> get2InitialPlacement() override;
    Action::PackedAction getMoveRobber() override;
    Action::PackedAction getTurnAction() override;
};
