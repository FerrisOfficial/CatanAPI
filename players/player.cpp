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

Action::PackedAction getDevAction(){
return 0; // Base implementation - should be overridden
}

Action::PackedAction getDiscardAction(){
return 0; // Base implementation - should be overridden
}

Action::PackedAction getMoveRobber(){
return 0; // Base implementation - should be overridden
}

Action::PackedAction getTurnAction(){
return 0; // Base implementation - should be overridden
}
