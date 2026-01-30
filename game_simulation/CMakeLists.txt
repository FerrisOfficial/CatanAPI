cmake_minimum_required(VERSION 3.10)
project(game_simulation)

# ----------------------------
# 🧱 Game simulation source files
# ----------------------------
set(GAME_SIM_SRC
  game.cpp
  board.cpp
  board_generate_actions.cpp
  ../utils/dumper.cpp
)

# ----------------------------
# 🔧 Game simulation library definition
# ----------------------------
add_library(game_simulation ${GAME_SIM_SRC})

target_include_directories(game_simulation PUBLIC
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/game_simulation
  ${CMAKE_SOURCE_DIR}/players
  ${CMAKE_SOURCE_DIR}/utils
)