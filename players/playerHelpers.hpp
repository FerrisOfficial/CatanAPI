#pragma once

#include <array>
#include <cstdint>
#include "../game_simulation/board.hpp"
#include "../game_simulation/player.hpp"
#include "../game_simulation/actions.hpp"
#include "../game_simulation/consts.hpp"

// Forward declarations
enum class PlayerId : uint8_t;

namespace PlayerHelpers {

// Basic utilities
uint8_t dice_pips(uint8_t diceNumber);

int effective_vp(const Board::BoardState* board, PlayerId pid);

inline int effective_vp(const Board::BoardState& board, PlayerId pid) {
    return effective_vp(&board, pid);
}

// Resource management
std::array<uint8_t, 5> unpack_resources(Player::PackedPlayer p);

uint8_t hand_count(const std::array<uint8_t, 5>& have);

std::array<uint8_t, 5> cost_for(BuyableType b);

uint16_t deficit(const std::array<uint8_t, 5>& have, const std::array<uint8_t, 5>& need);

// Board evaluation
int node_production_score(const Board::BoardState* board, NodeId nodeId);

int production_score_for_player(const Board::BoardState* board, PlayerId pid);

int settlement_potential_score(const Board::BoardState* board, PlayerId pid);

// Board queries
bool node_distance_rule_ok(const Board::BoardState* board, NodeId nodeId);

bool node_is_adjacent_to_own_road(const Board::BoardState* board, PlayerId pid, NodeId nodeId);

// Position evaluation
int evaluate_position(const Board::BoardState* board, PlayerId selfId);

// Action utilities
bool is_deterministic_action(Action::PackedAction a);

} // namespace PlayerHelpers
