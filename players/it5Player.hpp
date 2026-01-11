#pragma once

#include "it4Player.hpp"

// It5: It4 + smarter discard + hand-size (9) risk management.
struct It5Player : public It4Player {
    It5Player() : It4Player() {}
    virtual ~It5Player() = default;

    Action::PackedAction getDiscardAction() override;
    Action::PackedAction getTurnAction() override;
};
