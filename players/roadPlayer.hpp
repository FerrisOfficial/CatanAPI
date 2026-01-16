#pragma once

#include "it5Player.hpp"

#include <array>

// RoadPlayer: early placement de-prioritizes ore; midgame focuses on building one long continuous road.
struct RoadPlayer : public It5Player {
    RoadPlayer() : It5Player() {}
    virtual ~RoadPlayer() = default;

    std::pair<Action::PackedAction, Action::PackedAction> getInitialPlacement() override;
    std::pair<Action::PackedAction, Action::PackedAction> get2InitialPlacement() override;

    Action::PackedAction getDiscardAction() override;
    Action::PackedAction getTurnAction() override;

private:
    std::array<bool, 5> firstPlacementResources {false, false, false, false, false};
};
