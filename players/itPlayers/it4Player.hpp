#pragma once

#include "it3Player.hpp"

// It4: It3 turn/setup/robber + smarter dev-card usage.
struct It4Player : public It3Player {
    It4Player() : It3Player() {}
    virtual ~It4Player() = default;

    Action::PackedAction getDevAction() override;
    Action::PackedAction getTurnAction() override;
};
