#pragma once

#include "../player.hpp"

// ParaPlayer: heurystyczny gracz sterowany parametrami wczytywanymi z pliku.
// Domyślnie czyta konfigurację tylko z: ./players/paraPlayer.cfg (wyszukiwanego w górę po katalogach).
// Dla treningu równoległego można nadpisać ścieżkę przez zmienną środowiskową: CATAN_PARA_CFG.
struct ParaPlayer : public IPlayer {
    ParaPlayer();
    ~ParaPlayer() override = default;

    std::pair<Action::PackedAction, Action::PackedAction> getInitialPlacement() override;
    std::pair<Action::PackedAction, Action::PackedAction> get2InitialPlacement() override;
    Action::PackedAction getDevAction() override;
    Action::PackedAction getDiscardAction() override;
    Action::PackedAction getMoveRobber() override;
    Action::PackedAction getTurnAction() override;
};
