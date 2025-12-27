#include "player.hpp"
#include "game_simulation/board.hpp"
#include "utils/randomDevice.hpp"

struct RandomPlayer : public IPlayer {
    RandomPlayer() : IPlayer() {}

    std::pair<Action::PackedAction, Action::PackedAction> getInitialPlacement() override {
        auto allActions = Board::getAllInitialPlacementActions();
        int index = RandomDevice::getRandomInt(0, static_cast<int>(allActions.size()) - 1);
        return allActions[index];
    }

    std::pair<Action::PackedAction, Action::PackedAction> get2InitialPlacement() override {
        auto allActions = Board::getAll2InitialPlacementActions();
        int index = RandomDevice::getRandomInt(0, static_cast<int>(allActions.size()) - 1);
        return allActions[index];
    }

    Action::PackedAction getDevAction() override {
        auto allActions = Board::getAllDevCardActions();
        if (allActions.empty()) {
            return Action::PackedAction();
        }
        int index = RandomDevice::getRandomInt(0, static_cast<int>(allActions.size()) - 1);
        return allActions[index];
    }

    Action::PackedAction getDiscardAction() override {
        auto allActions = Board::getAllDiscardActions();
        int index = RandomDevice::getRandomInt(0, static_cast<int>(allActions.size()) - 1);
        return allActions[index];
    }

    Action::PackedAction getMoveRobber() override {
        auto allActions = Board::getAllRobberMoveActions();
        int index = RandomDevice::getRandomInt(0, static_cast<int>(allActions.size()) - 1);
        return allActions[index];
    }

    Action::PackedAction getTurnAction() override {
        auto allActions = Board::getAllTurnActions();
        int index = RandomDevice::getRandomInt(0, static_cast<int>(allActions.size()) - 1);
        return allActions[index];
    }
};
