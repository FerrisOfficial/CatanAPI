#pragma once

#include "it5Player.hpp"

#include <array>

// CityRushPlayer: prioritize grain/ore in initial placements and rush city upgrades.
struct CityRushPlayer : public It5Player {
    CityRushPlayer() : It5Player() {}
    virtual ~CityRushPlayer() = default;

    std::pair<Action::PackedAction, Action::PackedAction> getInitialPlacement() override;
    std::pair<Action::PackedAction, Action::PackedAction> get2InitialPlacement() override;
    Action::PackedAction getTurnAction() override;

private:
    bool hasFirstPlacement = false;
    std::array<bool, 5> firstPlacementResources {false, false, false, false, false};
};
