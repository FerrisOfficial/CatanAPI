#pragma once

#include "../itPlayers/it5Player.hpp"

#include <array>

// DevPlayer: It5 but heavily prioritizes buying development cards.
struct DevPlayer : public It5Player {
    DevPlayer() : It5Player() {}
    virtual ~DevPlayer() = default;

    std::pair<Action::PackedAction, Action::PackedAction> getInitialPlacement() override;
    std::pair<Action::PackedAction, Action::PackedAction> get2InitialPlacement() override;
    Action::PackedAction getDiscardAction() override;
    Action::PackedAction getTurnAction() override;

private:
    std::array<bool, 5> firstPlacementResources {false, false, false, false, false};
};
