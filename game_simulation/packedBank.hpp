#pragma once

#include <cstdint>
#include "consts.hpp"

// Packed bank representation (resources + development cards)
// Uses same bit widths as Player::PackedPlayer for compatibility.

namespace Bank {

using PackedBank = uint64_t;

// Resources: 5 bits each (same order as Player)
// - Brick:  bits 0-4
// - Lumber: bits 5-9
// - Wool:   bits 10-14
// - Grain:  bits 15-19
// - Ore:    bits 20-24

// Dev cards: (following same offsets as in Player)
// - Knight:       bits 25-28 (4 bits)
// - RoadBuilding: bits 29-30 (2 bits)
// - YearOfPlenty: bits 31-32 (2 bits)
// - Monopoly:     bits 33-34 (2 bits)
// - VictoryPoint: bits 35-37 (3 bits)

// Total dev cards field (cached): bits 38-42 (5 bits, max 31)
// This stores the sum of all development cards remaining in the bank.

// Create a new bank with a typical Catan supply
constexpr PackedBank makeNewBank() {
    PackedBank b = 0;
    // Typical resource supply in standard Catan: 19 of each resource
    b |= (PackedBank(19) << 0);   // Brick
    b |= (PackedBank(19) << 5);   // Lumber
    b |= (PackedBank(19) << 10);  // Wool
    b |= (PackedBank(19) << 15);  // Grain
    b |= (PackedBank(19) << 20);  // Ore

    // Dev cards: standard distribution
    // Knights: 14, RoadBuilding:2, YearOfPlenty:2, Monopoly:2, VictoryPoint:5
    b |= (PackedBank(14) << 25); // Knight
    b |= (PackedBank(2)  << 29); // RoadBuilding
    b |= (PackedBank(2)  << 31); // YearOfPlenty
    b |= (PackedBank(2)  << 33); // Monopoly
    b |= (PackedBank(5)  << 35); // VictoryPoint

    // Cached total dev cards (sum = 14+2+2+2+5 = 25)
    b |= (PackedBank(25) << 38);

    return b;
}

constexpr PackedBank packResource(PackedBank b, Resource r, uint8_t value) {
    uint8_t shift = static_cast<uint8_t>(r) * 5;
    b &= ~(PackedBank(0x1F) << shift);
    b |= (PackedBank(value & 0x1F) << shift);
    return b;
}
constexpr uint8_t unpackResource(PackedBank b, Resource r) {
    uint8_t shift = static_cast<uint8_t>(r) * 5;
    return static_cast<uint8_t>((b >> shift) & 0x1F);
}

constexpr PackedBank packDevCard(PackedBank b, DevType d, uint8_t value) {
    switch(d) {
        case DevType::Knight:       b &= ~(PackedBank(0xFULL) << 25); b |= PackedBank(value & 0xF) << 25; break;
        case DevType::RoadBuilding: b &= ~(PackedBank(0x3ULL) << 29); b |= PackedBank(value & 0x3) << 29; break;
        case DevType::YearOfPlenty: b &= ~(PackedBank(0x3ULL) << 31); b |= PackedBank(value & 0x3) << 31; break;
        case DevType::Monopoly:     b &= ~(PackedBank(0x3ULL) << 33); b |= PackedBank(value & 0x3) << 33; break;
        case DevType::VictoryPoint: b &= ~(PackedBank(0x7ULL) << 35); b |= PackedBank(value & 0x7) << 35; break;
        default: break;
    }
    return b;
}

constexpr uint8_t unpackDevCard(PackedBank b, DevType d) {
    switch(d) {
        case DevType::Knight:       return static_cast<uint8_t>((b >> 25) & 0xF);
        case DevType::RoadBuilding: return static_cast<uint8_t>((b >> 29) & 0x3);
        case DevType::YearOfPlenty: return static_cast<uint8_t>((b >> 31) & 0x3);
        case DevType::Monopoly:     return static_cast<uint8_t>((b >> 33) & 0x3);
        case DevType::VictoryPoint: return static_cast<uint8_t>((b >> 35) & 0x7);
        default: return 0;
    }
}

constexpr PackedBank packTotalDevCount(PackedBank b, uint8_t value) {
    b &= ~(PackedBank(0x1FULL) << 38);
    b |= (PackedBank(value & 0x1F) << 38);
    return b;
}
constexpr uint8_t unpackTotalDevCount(PackedBank b) {
    return static_cast<uint8_t>((b >> 38) & 0x1F);
}

constexpr uint8_t computeTotalDevCards(PackedBank b) {
    return static_cast<uint8_t>(
        unpackDevCard(b, DevType::Knight) +
        unpackDevCard(b, DevType::RoadBuilding) +
        unpackDevCard(b, DevType::YearOfPlenty) +
        unpackDevCard(b, DevType::Monopoly) +
        unpackDevCard(b, DevType::VictoryPoint)
    );
}

constexpr uint16_t totalResources(PackedBank b) {
    return static_cast<uint16_t>(
        unpackResource(b, Resource::Brick) +
        unpackResource(b, Resource::Lumber) +
        unpackResource(b, Resource::Wool) +
        unpackResource(b, Resource::Grain) +
        unpackResource(b, Resource::Ore)
    );
}

} // namespace Bank
