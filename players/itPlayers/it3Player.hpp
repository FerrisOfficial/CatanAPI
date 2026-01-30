#pragma once

#include "it2Player.hpp"

// It3: It2 turn logic + smarter setup + smarter robber.
struct It3Player : public It2Player {
    It3Player() : It2Player() {}
    virtual ~It3Player() = default;

    std::pair<Action::PackedAction, Action::PackedAction> getInitialPlacement() override;
    std::pair<Action::PackedAction, Action::PackedAction> get2InitialPlacement() override;
    Action::PackedAction getMoveRobber() override;

private:
    NodeId firstSettlementNode = 0xFF;
    std::array<bool, 5> firstPlacementResources {false, false, false, false, false};
};
