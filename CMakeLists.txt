cmake_minimum_required(VERSION 3.14)
project(CatanAPI)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include FetchContent module for downloading GoogleTest
include(FetchContent)

# Fetch GoogleTest
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Add include directory for game_simulation headers
include_directories(game_simulation)
include_directories(${CMAKE_SOURCE_DIR})
include_directories(players)

# Create game_simulation library
add_library(game_simulation
    players/player.cpp
    game_simulation/game.cpp
    game_simulation/board.cpp
)

target_include_directories(game_simulation PUBLIC
    ${CMAKE_SOURCE_DIR}/game_simulation
    ${CMAKE_SOURCE_DIR}/players
    ${CMAKE_SOURCE_DIR}
)

# Enable testing
enable_testing()

# Add subdirectory for tests
add_subdirectory(tests)