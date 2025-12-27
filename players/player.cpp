#include "player.hpp"

IPlayer::IPlayer() {
boardState = nullptr;
}

std::pair<Action::PackedAction, Action::PackedAction> IPlayer::getInitialPlacement() {
return 0; // Base implementation - should be overridden
}

std::pair<Action::PackedAction, Action::PackedAction> IPlayer::get2InitialPlacement() {
return 0; // Base implementation - should be overridden
}

Action::PackedAction IPlayer::getDevAction(){
return 0; // Base implementation - should be overridden
}

Action::PackedAction IPlayer::getDiscardAction(){
return 0; // Base implementation - should be overridden
}

Action::PackedAction IPlayer::getMoveRobber(){
return 0; // Base implementation - should be overridden
}

Action::PackedAction IPlayer::getTurnAction(){
return 0; // Base implementation - should be overridden
}
