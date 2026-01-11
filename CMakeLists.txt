cmake_minimum_required(VERSION 3.14)
project(CatanAPI)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ----------------------------
# 🧰 Diagnostics (optional)
# ----------------------------
option(ENABLE_ASAN "Enable AddressSanitizer (-fsanitize=address)" OFF)

if (ENABLE_ASAN)
  if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer -g)
    add_link_options(-fsanitize=address)
  else()
    message(WARNING "ENABLE_ASAN is ON but compiler '${CMAKE_CXX_COMPILER_ID}' may not support -fsanitize=address")
  endif()
endif()

include(FetchContent)

# ----------------------------
# 🧩 GoogleTest setup (needed for tests)
# ----------------------------
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# ----------------------------
# 📦 Include directories
# ----------------------------
include_directories(
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/game_simulation
  ${CMAKE_SOURCE_DIR}/players
)


# ----------------------------
# 🧱 Source files
# ----------------------------
set(GAME_SIM_SRC
  players/player.cpp
  players/randomPlayer.cpp
  players/playerHelpers.cpp
  game_simulation/game.cpp
  game_simulation/board.cpp
  game_simulation/board_generate_actions.cpp
  utils/dumper.cpp
)

# ----------------------------

# 🔧 Library definition
# ----------------------------
add_library(game_simulation ${GAME_SIM_SRC})

target_include_directories(game_simulation PUBLIC
  ${CMAKE_SOURCE_DIR}/game_simulation
  ${CMAKE_SOURCE_DIR}/players
  ${CMAKE_SOURCE_DIR}
)

# ----------------------------
# 🧪 Enable testing
# ----------------------------
enable_testing()
add_subdirectory(tests)

# ----------------------------
# ▶️ Add runs subdirectory
# ----------------------------
add_subdirectory(runs)
