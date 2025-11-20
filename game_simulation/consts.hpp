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

constexpr HexId HexIdNone = 0x1F;

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
    EndTurn = 2, // None

    // Robber
    MoveRobber = 3, // Arg1: HexId to move the robber to
    StealResource = 4, // Arg1: Resource type (0-4) 
    DiscardResources = 5, // Resources that player chose to discard

    // Buying/Building
    BuildRoad = 6, // Arg1: EdgeId to build road on
    BuildSettlement = 7, // Arg1: NodeId to build settlement on
    BuildCity = 8, // Arg1: NodeId to upgrade settlement to city
    BuyDevCard = 9, // None

    // Playing Development Cards
    PlayDevCardKnight = 10, // Arg1: HexId to move the robber to
    PlayDevCardRoadBuilding = 11, // Arg1: EdgeId to build first road on, Arg2: EdgeId to build second road on
    PlayDevCardYearOfPlenty = 12, // Agr1: Resource type (0-4) for first resource, Arg2: Resource type (0-4) for second resource
    PlayDevCardMonopoly = 13, // Arg1: Resource type (0-4)

    // Trading
    TradeBank = 14, // Arg1: Give bank resource type (0-4), Arg2: Take resource type (0-4), Arg3: trade ratio (2-4)
    ReceiveResources = 15, // Resources, Arg1: Resource type (0-4), Arg2: amount to give

    // Setup
    PlaceInitialSettlement = 16, // Arg1: NodeId to place settlement on
    Place2InitialSettlement = 17, // Arg1: NodeId to place settlement on
    PlaceInitialRoad = 18, // Arg1: EdgeId to place road on

    // //Undo Actions
    // UndoMoveRobber = 19, // Arg1: Previous HexId of the robber, Arg2: Resource type (0-4) stolen
    // UndoStealResource = 20, // Arg1: Resource type (0-4) that was stolen
    // UndoDiscardResources = 21, // Resources that player chose to discard

    // UndoBuildRoad = 22, // Arg1: EdgeId where road was built
    // UndoBuildSettlement = 23, // Arg1: NodeId where settlement was built
    // UndoBuildCity = 24, // Arg1: NodeId where city was built
    // UndoBuyDevCard = 25, // Arg1: DevType (0-4) of the dev card bought

    // UndoPlayDevCardKnight = 26, // Arg1: Previous HexId of the robber, Arg2: Resource type (0-4) stolen
    // UndoPlayDevCardRoadBuilding = 27, // Arg1: EdgeId where first road was built, Arg2: EdgeId where second road was built
    // UndoPlayDevCardYearOfPlenty = 28, // Arg1: Resource type (0-4) for first resource, Arg2: Resource type (0-4) for second resource
    // UndoPlayDevCardMonopoly = 29, // Arg1: Resource type (0-4) stolen, Arg2: amount stolen

    // UndoTradeBank = 30, // Arg1: Give bank resource type (0-4), Arg2: Take resource type (0-4), Arg3: trade ratio (2-4)
    // UndoReceiveResources = 31, // Resources, Arg1: Resource type (0-4), Arg2: amount to take back

    // UndoPlaceInitialSettlement = 32, // Arg1: NodeId where settlement was placed
    // UndoPlace2InitialSettlement = 33, // Arg1: NodeId where settlement was placed
    // UndoPlaceInitialRoad = 34, // Arg1: EdgeId where road was placed
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