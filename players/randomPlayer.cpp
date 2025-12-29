#include "randomPlayer.hpp"
#include "game_simulation/board.hpp"
#include "utils/randomDevice.hpp"
#include "utils/logger.hpp"

std::pair<Action::PackedAction, Action::PackedAction> RandomPlayer::getInitialPlacement() {
    auto actions = boardState->generatePlaceInitialStructures(boardState->currentPlayer);
    logger.log("Generated ", std::to_string(actions.size()), " initial placement actions.");
    if (actions.empty()) logger.error("No initial placement actions available!");
    int idx = RandomDevice::uniform_u32_range(0, static_cast<uint32_t>(actions.size()) - 1);
    return {actions[idx], actions[idx]};
}

std::pair<Action::PackedAction, Action::PackedAction> RandomPlayer::get2InitialPlacement() {
    auto actions = boardState->generatePlace2InitialStructures(boardState->currentPlayer);
    if (actions.empty()) logger.error("No 2nd initial placement actions available!");
    int idx = RandomDevice::uniform_u32_range(0, static_cast<uint32_t>(actions.size()) - 1);
    return {actions[idx], actions[idx]};
}

Action::PackedAction RandomPlayer::getDevAction() {
    auto action = Action::getEmptyAction();
    action = Action::packType(action, ActionType::NoAction);
    return action;
}

Action::PackedAction RandomPlayer::getDiscardAction() {
    return Action::PackedAction();
}

Action::PackedAction RandomPlayer::getMoveRobber() {
    return Action::PackedAction();
}

Action::PackedAction RandomPlayer::getTurnAction() {
    auto actions = boardState->getLegalActions(boardState->currentPlayer);
    // print all legal actions
    if (actions.empty()) {
        logger.error("No legal actions available for current player!");
        return Action::getEmptyAction();
    }
    for (const auto& action : actions) {
        logger.log("Legal action: ", actionTypeName(Action::unpackType(action)));
    }
    auto noAction = Action::getEmptyAction();
    int idx = RandomDevice::uniform_u32_range(0, static_cast<uint32_t>(actions.size()) - 1);
    return actions[idx];
}
