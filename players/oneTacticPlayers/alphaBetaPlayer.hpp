#pragma once

#include "itPlayers/it5Player.hpp"

struct alphaBetaPlayer : public It5Player {
    alphaBetaPlayer() : It5Player() {}
    virtual ~alphaBetaPlayer() = default;

    Action::PackedAction getTurnAction() override;
};
