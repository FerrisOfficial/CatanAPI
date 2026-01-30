#pragma once

#include "players/randomPlayer.hpp"

struct It1Player : public RandomPlayer {
    It1Player() : RandomPlayer() {}
    virtual ~It1Player() = default;
    Action::PackedAction getTurnAction() override;
};
