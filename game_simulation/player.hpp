#pragma once

#include <cstdint>
#include "consts.hpp"

// Packed 64-bit player state representation
// Layout (from LSB to MSB)
//
// Resources (5 bits each, 25 bits total):
// - Brick:  bits 0-4   (5 bits, max 31)
// - Lumber: bits 5-9   (5 bits, max 31)  
// - Wool:   bits 10-14 (5 bits, max 31)
// - Grain:  bits 15-19 (5 bits, max 31)
// - Ore:    bits 20-24 (5 bits, max 31)
//
// Development Cards (13 bits total):
// - Knight:       bits 25-28 (4 bits, max 15)
// - RoadBuilding: bits 29-30 (2 bits, max 3)
// - YearOfPlenty: bits 31-32 (2 bits, max 3)
// - Monopoly:     bits 33-34 (2 bits, max 3)
// - VictoryPoint: bits 35-37 (3 bits, max 7)
//
// Game State:
// - Used Knights:        bits 38-41 (4 bits, max 15)
// - Longest Road Length: bits 42-45 (4 bits, max 15)
// - Longest Road Flag:   bit 46     (1 bit, boolean)
// - Largest Army Flag:   bit 47     (1 bit, boolean)
//
// Available structures:
// - Settlements:         bits 48-50 (3 bits, max 8)
// - Cities:              bits 51-53 (3 bits, max 8)
// - Roads:               bits 54-57 (4 bits, max 15)
//
// Victory Points:        bits 58-62 (5 bits, max 32)

namespace Player {

using PackedPlayer = uint64_t;

constexpr PackedPlayer makeNewPlayer() {
    return (5ULL << 48) | (4ULL << 51) | (15ULL << 54);
}

constexpr PackedPlayer packResource(PackedPlayer p, Resource r, uint8_t value) {
    p &= ~(0x1FULL << (r*5));
    p |= (uint64_t(value & 0x1F) << (r*5));
    return p;
}
constexpr uint8_t unpackResource(PackedPlayer p, Resource r) {
    return (p >> (r*5)) & 0x1F;
}

constexpr PackedPlayer packDevCard(PackedPlayer p, DevType d, uint8_t value) {
    switch(d){
        case DevType::Knight:       p &= ~(0xFULL << 25); p |= uint64_t(value & 0xF) << 25; break;  // 4 bits
        case DevType::RoadBuilding: p &= ~(0x3ULL << 29); p |= uint64_t(value & 0x3) << 29; break;  // 2 bits
        case DevType::YearOfPlenty: p &= ~(0x3ULL << 31); p |= uint64_t(value & 0x3) << 31; break;  // 2 bits
        case DevType::Monopoly:     p &= ~(0x3ULL << 33); p |= uint64_t(value & 0x3) << 33; break;  // 2 bits
        case DevType::VictoryPoint: p &= ~(0x7ULL << 35); p |= uint64_t(value & 0x7) << 35; break;  // 3 bits
    }
    return p;
}
constexpr uint8_t unpackDevCard(PackedPlayer p, DevType d) {
    switch(d){
        case DevType::Knight:       return (p >> 25) & 0xF;
        case DevType::RoadBuilding: return (p >> 29) & 0x3;
        case DevType::YearOfPlenty: return (p >> 31) & 0x3;
        case DevType::Monopoly:     return (p >> 33) & 0x3;
        case DevType::VictoryPoint: return (p >> 35) & 0x7;
        default: return 0;
    }
}

constexpr PackedPlayer packUsedKnights(PackedPlayer p, uint8_t value) { return (p & ~(0xFULL << 38)) | (uint64_t(value & 0xF) << 38); }
constexpr uint8_t unpackUsedKnights(PackedPlayer p) { return (p >> 38) & 0xF; }

constexpr PackedPlayer packLongestRoadLength(PackedPlayer p, uint8_t value) { return (p & ~(0xFULL << 42)) | (uint64_t(value & 0xF) << 42); }
constexpr uint8_t unpackLongestRoadLength(PackedPlayer p) { return (p >> 42) & 0xF; }

constexpr PackedPlayer packLongestRoadFlag(PackedPlayer p, bool v) { return (p & ~(1ULL << 46)) | (uint64_t(v) << 46); }
constexpr bool unpackLongestRoadFlag(PackedPlayer p) { return (p >> 46) & 1; }

constexpr PackedPlayer packLargestArmyFlag(PackedPlayer p, bool v) { return (p & ~(1ULL << 47)) | (uint64_t(v) << 47); }
constexpr bool unpackLargestArmyFlag(PackedPlayer p) { return (p >> 47) & 1; }

constexpr PackedPlayer packAvailableStructures(PackedPlayer p, StructureType s, uint8_t value) {
    switch(s) {
        case StructureType::Settlement: p &= ~(0x7ULL << 48); p |= (uint64_t(value & 0x7) << 48); break;
        case StructureType::City:       p &= ~(0x7ULL << 51); p |= (uint64_t(value & 0x7) << 51); break;
        case StructureType::Road:       p &= ~(0xFULL << 54); p |= (uint64_t(value & 0xF) << 54); break;
    }
    return p;
}
constexpr uint8_t unpackAvailableStructures(PackedPlayer p, StructureType s) {
    switch(s) {
        case StructureType::Settlement: return (p >> 48) & 0x7;
        case StructureType::City:       return (p >> 51) & 0x7;
        case StructureType::Road:       return (p >> 54) & 0xF;
        default: return 0;
    }   
}

constexpr PackedPlayer packVictoryPoints(PackedPlayer p, uint8_t value) { return (p & ~(0x1FULL << 58)) | (uint64_t(value & 0x1F) << 58); }
constexpr uint8_t unpackVictoryPoints(PackedPlayer p) { return (p >> 58) & 0x1F; }

constexpr bool hasEnoughResources(PackedPlayer p, BuyableType b) {
    const auto& cost = StructureCost[static_cast<size_t>(b)];
    return unpackResource(p, Resource::Brick) >= cost[0] &&
           unpackResource(p, Resource::Lumber) >= cost[1] &&
           unpackResource(p, Resource::Wool) >= cost[2] &&
           unpackResource(p, Resource::Grain) >= cost[3] &&
           unpackResource(p, Resource::Ore) >= cost[4];
}

constexpr void buy(PackedPlayer &p, BuyableType b, DevType d = DevType::NoDev) {
    const auto& cost = StructureCost[static_cast<size_t>(b)];
    p = packResource(p, Resource::Brick, unpackResource(p, Resource::Brick) - cost[0]);
    p = packResource(p, Resource::Lumber, unpackResource(p, Resource::Lumber) - cost[1]);
    p = packResource(p, Resource::Wool, unpackResource(p, Resource::Wool) - cost[2]);
    p = packResource(p, Resource::Grain, unpackResource(p, Resource::Grain) - cost[3]);
    p = packResource(p, Resource::Ore, unpackResource(p, Resource::Ore) - cost[4]);

    switch (b)
    {
    case BuyableType::Road:
        p = packAvailableStructures(p, StructureType::Road, unpackAvailableStructures(p, StructureType::Road) - 1);
        break;
    case BuyableType::Settlement:
        p = packAvailableStructures(p, StructureType::Settlement, unpackAvailableStructures(p, StructureType::Settlement) - 1);
        p = packVictoryPoints(p, unpackVictoryPoints(p) + 1);
        break;
    case BuyableType::City:
        p = packAvailableStructures(p, StructureType::City, unpackAvailableStructures(p, StructureType::City) - 1);
        p = packVictoryPoints(p, unpackVictoryPoints(p) + 1);
        p = packAvailableStructures(p, StructureType::Settlement, unpackAvailableStructures(p, StructureType::Settlement) + 1);
        break;
    case BuyableType::DevCard:
        p = packDevCard(p, d, unpackDevCard(p, d) + 1);
        break;
    default:
        break;
    }
}

} // namespace Player