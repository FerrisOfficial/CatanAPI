#pragma once

#include <cstdint>
#include <array>

constexpr int NODE_COUNT = 54; 
constexpr int EDGE_COUNT = 72; 
constexpr int HEX_COUNT = 19; 

using NodeId = uint8_t;
using EdgeId = uint8_t;
using HexId = uint8_t; 
using DiceNumber = uint8_t;

constexpr HexId HexIdNone = 20; 

enum class PlayerId : uint8_t { 
    Player0 = 0, 
    Player1 = 1,
    NoPlayer = 2
};

enum Resource : uint8_t { 
    Brick = 0, 
    Lumber = 1, 
    Wool = 2, 
    Grain = 3, 
    Ore = 4,
    NoResource = 5 
}; 

enum class DevType : uint8_t { 
    Knight = 0, 
    RoadBuilding = 1, 
    YearOfPlenty = 2, 
    Monopoly = 3, 
    VictoryPoint = 4,
    NoDev = 5
};

enum class StructureType : uint8_t {
    Road = 0,
    Settlement = 1,
    City = 2,
    NoStructure = 3
};

enum class BuyableType : uint8_t {
    Road = 0,
    Settlement = 1,
    City = 2,
    DevCard = 3
};

constexpr std::array<std::array<uint8_t,5>,4> StructureCost {{
    {1,1,0,0,0}, // Road
    {1,1,1,1,0}, // Settlement
    {0,0,0,2,3},  // City
    {0,0,1,1,1}   // DevCard
}};

enum class ActionType : uint8_t {
    // Turn flow
    RollDice = 1, // Arg1: dice value (2-12)
    EndTurn = 2,

    // Robber
    MoveRobber = 3, // Arg1: HexId to move the robber to, Arg2: Resource type (0-4) 
    DiscardResources = 4, // Resources

    // Buying/Building
    BuildRoad = 5, // Arg1: EdgeId to build road on
    BuildSettlement = 6, // Arg1: NodeId to build settlement on
    BuildCity = 7, // Arg1: NodeId to upgrade settlement to city
    BuyDevCard = 8,

    // Playing Development Cards
    PlayDevCardKnight = 9, // Arg1: HexId to move the robber to, Arg2: Resource type (0-4)
    PlayDevCardRoadBuilding = 10, // Arg1: EdgeId to build first road on, Arg2: EdgeId to build second road on
    PlayDevCardYearOfPlenty = 11, // Resources
    PlayDevCardMonopoly = 12, // Arg1: Resource type (0-4)

    // Trading
    TradeBank = 12, // Resources, Arg1: Resource type (0-4)
    ReceiveResources = 13, // Resources

    // Setup
    PlaceInitialSettlement = 14,
    PlaceInitialRoad = 15,
};

enum class PortType : uint8_t {
    ThreeForOne = 0,
    BrickPort = 1,
    LumberPort = 2,
    WoolPort = 3,
    GrainPort = 4,
    OrePort = 5,
    NoPort = 6
};