cmake_minimum_required(VERSION 3.10)
project(players)

# ----------------------------
# 🧱 Player source files
# ----------------------------
set(PLAYER_SRC
  player.cpp
  randomPlayer.cpp
  parametricPlayers/paraPlayer.cpp
  parametricPlayers/paraSetit5Player.cpp
  oneTacticPlayers/devPlayer.cpp
  oneTacticPlayers/alphaBetaPlayer.cpp
  oneTacticPlayers/oneResourcePlayer.cpp
  oneTacticPlayers/cityRushPlayer.cpp
  roadPlayer.cpp
  playerHelpers.cpp
  itPlayers/it1Player.cpp
  itPlayers/it2Player.cpp
  itPlayers/it3Player.cpp
  itPlayers/it4Player.cpp
  itPlayers/it5Player.cpp
)

# ----------------------------
# 🔧 Player library definition
# ----------------------------
add_library(players ${PLAYER_SRC})

target_include_directories(players PUBLIC
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/game_simulation
  ${CMAKE_SOURCE_DIR}/players
  ${CMAKE_SOURCE_DIR}/utils
)

target_link_libraries(players PUBLIC game_simulation)