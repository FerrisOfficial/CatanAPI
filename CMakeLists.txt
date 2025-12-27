cmake_minimum_required(VERSION 3.14)
project(CatanAPI)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ----------------------------
# 🟩 Define option early
# ----------------------------
option(ENABLE_DISPLAY "Enable active display (displayActive)" ON)

include(FetchContent)

# ----------------------------
# 🧩 GoogleTest setup
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
  ${CMAKE_SOURCE_DIR}/display
)

# ----------------------------
# 🧱 Source files
# ----------------------------
set(GAME_SIM_SRC
  players/player.cpp
  game_simulation/game.cpp
  game_simulation/board.cpp
  game_simulation/board_generate_actions.cpp
)

if(ENABLE_DISPLAY)
  list(APPEND GAME_SIM_SRC display/displayActive.cpp)
  message(STATUS "Display enabled: including displayActive.cpp")
else()
  message(STATUS "Display disabled: only using displayNotActive.hpp (no .cpp added)")
endif()

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
# 🎨 SFML (only if display enabled)
# ----------------------------
if(ENABLE_DISPLAY)
  target_compile_definitions(game_simulation PUBLIC DISPLAY)

  FetchContent_Declare(
    sfml
    URL https://github.com/SFML/SFML/archive/refs/tags/2.6.1.zip
  )
  set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(SFML_BUILD_DOC OFF CACHE BOOL "" FORCE)
  set(SFML_BUILD_TEST_SUITE OFF CACHE BOOL "" FORCE)
  set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(sfml)

  target_link_libraries(game_simulation PRIVATE sfml-graphics sfml-window sfml-system)
  target_compile_definitions(game_simulation PRIVATE SFML_STATIC)
  message(STATUS "Building with DISPLAY enabled (SFML fetched and linked)")
else()
  message(STATUS "Building with DISPLAY disabled (header-only stub)")
endif()

# ----------------------------
# 🎨 clang-format
# ----------------------------
find_program(CLANG_FORMAT_EXE
  NAMES clang-format clang-format-17 clang-format-16
)

if(CLANG_FORMAT_EXE)
  file(GLOB_RECURSE CLANG_FORMAT_FILES
    ${CMAKE_SOURCE_DIR}/*.cpp
    ${CMAKE_SOURCE_DIR}/*.h
    ${CMAKE_SOURCE_DIR}/*.hpp
    ${CMAKE_SOURCE_DIR}/*.c
  )

  add_custom_target(clang-format
    COMMAND ${CLANG_FORMAT_EXE}
            -i
            --style=file
            ${CLANG_FORMAT_FILES}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Running clang-format on all source files"
  )

  add_custom_target(clang-format-check
    COMMAND ${CLANG_FORMAT_EXE}
            --dry-run
            --Werror
            --style=file
            ${CLANG_FORMAT_FILES}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Checking clang-format compliance"
  )
endif()

# ----------------------------
# 🧪 Testing
# ----------------------------
enable_testing()
add_subdirectory(tests)
