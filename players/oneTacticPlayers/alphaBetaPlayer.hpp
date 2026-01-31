#pragma once

#include "itPlayers/it5Player.hpp"

struct AlphaBetaPlayer : public It5Player {
    AlphaBetaPlayer() : It5Player() {}
    virtual ~AlphaBetaPlayer() = default;

    Action::PackedAction getTurnAction() override;
};
