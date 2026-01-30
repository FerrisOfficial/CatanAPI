#include "game_simulation/board.hpp"
#include "randomPlayer.hpp"
#include "utils/randomDevice.hpp"

#include <algorithm>

std::pair<Action::PackedAction, Action::PackedAction> RandomPlayer::getInitialPlacement() {
    auto actions = boardState->generatePlaceInitialStructures(boardState->currentPlayer);
    if (actions.empty()) {
        auto noAction = Action::getEmptyAction();
        return {noAction, noAction};
    }
    int idx = RandomDevice::uniform_u32_range(0, static_cast<uint32_t>(actions.size()) - 1);
    return {actions[idx], actions[idx]};
}

std::pair<Action::PackedAction, Action::PackedAction> RandomPlayer::get2InitialPlacement() {
    auto actions = boardState->generatePlace2InitialStructures(boardState->currentPlayer);
    if (actions.empty()) {
        auto noAction = Action::getEmptyAction();
        return {noAction, noAction};
    }
    int idx = RandomDevice::uniform_u32_range(0, static_cast<uint32_t>(actions.size()) - 1);
    return {actions[idx], actions[idx]};
}

Action::PackedAction RandomPlayer::getDevAction() {
    auto actions = boardState->generatePlayDevCardActions(boardState->currentPlayer);
    actions.push_back(Action::getEmptyAction()); // No action option
    int idx = RandomDevice::uniform_u32_range(0, static_cast<uint32_t>(actions.size()) - 1);
    return actions[idx];
}

Action::PackedAction RandomPlayer::getDiscardAction() {
    auto action = Action::getEmptyAction();
    uint8_t totalResources = Player::totalResources(boardState->packedPlayers[static_cast<uint8_t>(boardState->currentPlayer)]);
    uint8_t toDiscard = totalResources / 2;
    std::vector<Resource> resourcePool;

    for (Resource r : {
            Resource::Lumber,
            Resource::Brick,
            Resource::Wool,
            Resource::Grain,
            Resource::Ore
        }) {

        uint8_t count = Player::unpackResource(boardState->packedPlayers[static_cast<uint8_t>(boardState->currentPlayer)], r);
        for (uint8_t i = 0; i < count; ++i) {
            resourcePool.push_back(r);
        }
    }

    std::shuffle(resourcePool.begin(), resourcePool.end(), RandomDevice::get_rng());
    for (uint8_t i = 0; i < toDiscard; ++i) {
        action = Action::packResource(action, resourcePool[i], Action::unpackResource(action, resourcePool[i]) + 1);
    }

    return action;
}

Action::PackedAction RandomPlayer::getMoveRobber() {
    auto actions = boardState->generateMoveRobberActions(boardState->currentPlayer);
    if (actions.empty()) {
        return Action::getEmptyAction();
    }
    int idx = RandomDevice::uniform_u32_range(0, static_cast<uint32_t>(actions.size()) - 1);
    return actions[idx];
}

Action::PackedAction RandomPlayer::getTurnAction() {
    auto actions = boardState->getLegalActions(boardState->currentPlayer);
    if (actions.empty()) {
        return Action::getEmptyAction();
    }
    int idx = RandomDevice::uniform_u32_range(0, static_cast<uint32_t>(actions.size()) - 1);
    return actions[idx];
}
