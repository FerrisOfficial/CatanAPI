# CatanAPI
C++ Catan API for bot fights

## Overview
This project provides a high-performance C++ API for Settlers of Catan game simulation, designed for bot development and automated gameplay. The API uses efficient bit-packing techniques to represent game state and actions in a compact format.

## Features
- **Packed Data Structures**: Efficient 64-bit player representation and 32-bit action encoding
- **Comprehensive Testing**: Full test coverage using Google Test framework
- **CMake Build System**: Modern CMake configuration with automatic dependency management
- **Cross-Platform**: Compatible with Windows, Linux, and macOS

## Project Structure
```
CatanAPI/
├── game_simulation/          # Core game logic headers
│   ├── player.hpp           # Player state packing/unpacking functions
│   ├── actions.hpp          # Action encoding/decoding functions
│   ├── consts.hpp           # Game constants and enums
│   └── topology.cpp         # Game board topology
├── tests/                   # Google Test test suites
│   ├── test_player.cpp      # Player functionality tests
│   ├── test_actions.cpp     # Action functionality tests
│   └── CMakeLists.txt       # Test configuration
├── build/                   # Build artifacts (generated)
├── CMakeLists.txt           # Main CMake configuration
└── README.md                # This file
```

## Building the Project

### Prerequisites
- **CMake** 3.14 or higher
- **C++17** compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- **Internet connection** (for Google Test download during first build)

### Build Instructions

#### Windows (MinGW)
```bash
mkdir build
cd build
cmake -G "MinGW Makefiles" ..
cmake --build .
```

#### Linux/macOS
```bash
mkdir build
cd build
cmake ..
make -j$(nproc)
```

#### Visual Studio (Windows)
```bash
mkdir build
cd build
cmake -G "Visual Studio 17 2022" ..
cmake --build . --config Release
```

## Running Tests

The project includes comprehensive test suites for all major components:

### Run All Tests
```bash
# From build directory
ctest --output-on-failure
```

### Run Individual Test Suites
```bash
# Player functionality tests (10 tests)
./tests/player_tests.exe    # Windows
./tests/player_tests        # Linux/macOS

# Action functionality tests (12 tests)
./tests/actions_tests.exe   # Windows  
./tests/actions_tests       # Linux/macOS
```

### Test Coverage
- **Player Tests** (10 tests): Resource packing, development cards, flags, boundary conditions
- **Actions Tests** (12 tests): Action encoding, player IDs, game scenarios, overflow handling
- **Total**: 22 tests covering all public API functions

## API Documentation

### Core Data Types

#### PackedPlayer (64-bit)
Efficiently stores complete player state:
- **Resources** (5 types × 5 bits = 25 bits): Brick, Lumber, Wool, Grain, Ore
- **Development Cards** (5 types, variable bits): Knight, Road Building, Year of Plenty, Monopoly, Victory Point
- **Game State**: Used knights, longest road length, achievement flags

#### PackedAction (32-bit)
Encodes game actions:
- **Action Type** (4 bits): RollDice, BuildRoad, BuildSettlement, etc.
- **Player ID** (2 bits): Player1, Player2, NoPlayer
- **Arguments** (2 × 8 bits): Context-specific parameters
- **Value** (8 bits): Quantity or additional data

### Example Usage

```cpp
#include "player.hpp"
#include "actions.hpp"

// Create a player with resources
PackedPlayer player = 0;
player = packResource(player, Brick, 3);
player = packResource(player, Lumber, 2);
player = packDevCard(player, Knight, 1);

// Verify resources
assert(unpackResource(player, Brick) == 3);
assert(unpackResource(player, Lumber) == 2);
assert(unpackDevCard(player, Knight) == 1);

// Create an action
PackedAction action = packAction(BuildRoad, Player1, 15); // Build road on edge 15
assert(unpackType(action) == BuildRoad);
assert(unpackPlayer(action) == Player1);
assert(unpackArg1(action) == 15);
```

### Constants and Enums

- **Game Board**: 54 nodes, 72 edges, 19 hexes
- **Players**: Player1, Player2, NoPlayer  
- **Resources**: Brick, Lumber, Wool, Grain, Ore, NoResource
- **Actions**: RollDice, BuildRoad, BuildSettlement, BuildCity, MoveRobber, StealCard
- **Development Cards**: Knight, RoadBuilding, YearOfPlenty, Monopoly, VictoryPoint

## Development

### Adding New Tests
1. Add test functions to `tests/test_player.cpp` or `tests/test_actions.cpp`
2. Follow Google Test conventions (`TEST_F` macros)
3. Rebuild and run tests to verify

### Code Style
- Use `constexpr` for compile-time evaluation where possible
- Follow bit-packing patterns for new data structures
- Maintain comprehensive test coverage for new features

## Contributing
1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Add tests for new functionality
4. Ensure all tests pass (`ctest`)
5. Commit your changes (`git commit -am 'Add amazing feature'`)
6. Push to the branch (`git push origin feature/amazing-feature`)
7. Open a Pull Request

## License
This project is licensed under the MIT License - see the LICENSE file for details.