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
    PackedBank pb = 0;
    // Typical resource supply in standard Catan: 19 of each resource
    pb |= (PackedBank(19) << 0);   // Brick
    pb |= (PackedBank(19) << 5);   // Lumber
    pb |= (PackedBank(19) << 10);  // Wool
    pb |= (PackedBank(19) << 15);  // Grain
    pb |= (PackedBank(19) << 20);  // Ore

    // Dev cards: standard distribution
    // Knights: 14, RoadBuilding:2, YearOfPlenty:2, Monopoly:2, VictoryPoint:5
    pb |= (PackedBank(14) << 25); // Knight
    pb |= (PackedBank(2)  << 29); // RoadBuilding
    pb |= (PackedBank(2)  << 31); // YearOfPlenty
    pb |= (PackedBank(2)  << 33); // Monopoly
    pb |= (PackedBank(5)  << 35); // VictoryPoint

    // Cached total dev cards (sum = 14+2+2+2+5 = 25)
    pb |= (PackedBank(25) << 38);

    return pb;
}

constexpr PackedBank packResource(PackedBank pb, Resource r, uint8_t value) {
    uint8_t shift = static_cast<uint8_t>(r) * 5;
    pb &= ~(PackedBank(0x1F) << shift);
    pb |= (PackedBank(value & 0x1F) << shift);
    return pb;
}
constexpr uint8_t unpackResource(PackedBank pb, Resource r) {
    uint8_t shift = static_cast<uint8_t>(r) * 5;
    return static_cast<uint8_t>((pb >> shift) & 0x1F);
}

constexpr PackedBank packDevCard(PackedBank pb, DevType d, uint8_t value) {
    switch(d) {
        case DevType::Knight:       pb &= ~(PackedBank(0xFULL) << 25); pb |= PackedBank(value & 0xF) << 25; break;
        case DevType::RoadBuilding: pb &= ~(PackedBank(0x3ULL) << 29); pb |= PackedBank(value & 0x3) << 29; break;
        case DevType::YearOfPlenty: pb &= ~(PackedBank(0x3ULL) << 31); pb |= PackedBank(value & 0x3) << 31; break;
        case DevType::Monopoly:     pb &= ~(PackedBank(0x3ULL) << 33); pb |= PackedBank(value & 0x3) << 33; break;
        case DevType::VictoryPoint: pb &= ~(PackedBank(0x7ULL) << 35); pb |= PackedBank(value & 0x7) << 35; break;
        default: break;
    }
    return pb;
}

constexpr uint8_t unpackDevCard(PackedBank pb, DevType d) {
    switch(d) {
        case DevType::Knight:       return static_cast<uint8_t>((pb >> 25) & 0xF);
        case DevType::RoadBuilding: return static_cast<uint8_t>((pb >> 29) & 0x3);
        case DevType::YearOfPlenty: return static_cast<uint8_t>((pb >> 31) & 0x3);
        case DevType::Monopoly:     return static_cast<uint8_t>((pb >> 33) & 0x3);
        case DevType::VictoryPoint: return static_cast<uint8_t>((pb >> 35) & 0x7);
        default: return 0;
    }
}

constexpr PackedBank packTotalDevCount(PackedBank pb, uint8_t value) {
    pb &= ~(PackedBank(0x1FULL) << 38);
    pb |= (PackedBank(value & 0x1F) << 38);
    return pb;
}
constexpr uint8_t unpackTotalDevCount(PackedBank pb) {
    return static_cast<uint8_t>((pb >> 38) & 0x1F);
}

constexpr uint8_t computeTotalDevCards(PackedBank pb) {
    return static_cast<uint8_t>(
        unpackDevCard(pb, DevType::Knight) +
        unpackDevCard(pb, DevType::RoadBuilding) +
        unpackDevCard(pb, DevType::YearOfPlenty) +
        unpackDevCard(pb, DevType::Monopoly) +
        unpackDevCard(pb, DevType::VictoryPoint)
    );
}

constexpr PackedBank buyableTransaction(PackedBank pb, BuyableType b, DevType d = DevType::NoDev, bool sell = true) {
    const auto& cost = StructureCost[static_cast<size_t>(b)];

    if (sell) {
        // Bank sells an item to a player: add resources from bank
        pb = packResource(pb, Resource::Brick, unpackResource(pb, Resource::Brick) + cost[0]);
        pb = packResource(pb, Resource::Lumber, unpackResource(pb, Resource::Lumber) + cost[1]);
        pb = packResource(pb, Resource::Wool, unpackResource(pb, Resource::Wool) + cost[2]);
        pb = packResource(pb, Resource::Grain, unpackResource(pb, Resource::Grain) + cost[3]);
        pb = packResource(pb, Resource::Ore, unpackResource(pb, Resource::Ore) + cost[4]);
        if (b == BuyableType::DevCard) {
            if (d != DevType::NoDev) {
                uint8_t cur = unpackDevCard(pb, d);
                pb = packDevCard(pb, d, cur - 1);
            }
            // Update cached total after possible change
            pb = packTotalDevCount(pb, computeTotalDevCards(pb));
        }
    } else {
        // Bank receives an item back and gives resources to player
        pb = packResource(pb, Resource::Brick, unpackResource(pb, Resource::Brick) - cost[0]);
        pb = packResource(pb, Resource::Lumber, unpackResource(pb, Resource::Lumber) - cost[1]);
        pb = packResource(pb, Resource::Wool, unpackResource(pb, Resource::Wool) - cost[2]);
        pb = packResource(pb, Resource::Grain, unpackResource(pb, Resource::Grain) - cost[3]);
        pb = packResource(pb, Resource::Ore, unpackResource(pb, Resource::Ore) - cost[4]);

        if (b == BuyableType::DevCard) {
            if (d != DevType::NoDev) {
                uint8_t cur = unpackDevCard(pb, d);
                pb = packDevCard(pb, d, cur + 1);
            }
            // Update cached total after possible change
            pb = packTotalDevCount(pb, computeTotalDevCards(pb));
        }
    }

    return pb;
}

constexpr void changeResourceQuantity(PackedBank &pb, Resource r, int8_t delta) {
    pb = packResource(pb, r, unpackResource(pb, r) + delta);
}

} // namespace Bank