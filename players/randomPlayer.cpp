#include "player.hpp"
#include "game_simulation/board.hpp"
#include "game_simulation/randomDevice.hpp"

struct RandomPlayer : public IPlayer {
    RandomPlayer() : IPlayer() {}

    // std::pair<Action::PackedAction, Action::PackedAction> getInitialPlacement() override {
    //     return RandomDevice::getInitialPlacement(*this);
    // }
};
