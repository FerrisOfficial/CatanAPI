#pragma once

#include "it5Player.hpp"

// It5: It5 + alpha-beta pruning for turn action
struct alphaBetaPlayer : public It5Player {
    alphaBetaPlayer() : It5Player() {}
    virtual ~alphaBetaPlayer() = default;

    Action::PackedAction getTurnAction() override;
};
