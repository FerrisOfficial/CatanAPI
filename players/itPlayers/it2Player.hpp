#pragma once

#include "players/randomPlayer.hpp"

struct It2Player : public RandomPlayer {
    It2Player() : RandomPlayer() {}
    virtual ~It2Player() = default;
    Action::PackedAction getTurnAction() override;
};
