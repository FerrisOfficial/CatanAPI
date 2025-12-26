#include "player.hpp"

IPlayer::IPlayer() {
boardState = nullptr;
}

Action::PackedAction IPlayer::getInitialSettlement() {
return 0; // Base implementation - should be overridden
}

Action::PackedAction IPlayer::getInitialRoad() {
return 0; // Base implementation - should be overridden
}

Action::PackedAction IPlayer::get2InitialSettlement() {
return 0; // Base implementation - should be overridden
}

Action::PackedAction IPlayer::get2InitialRoad() {
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
