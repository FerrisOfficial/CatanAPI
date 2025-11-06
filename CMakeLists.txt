cmake_minimum_required(VERSION 3.14)
project(CatanAPI)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include FetchContent module for downloading GoogleTest
include(FetchContent)

# Fetch GoogleTest
# Build dependencies as static libraries to avoid missing DLLs during test discovery on Windows
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
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

if(ENABLE_DISPLAY)
    list(APPEND GAME_SIM_SRC display/displayActive.cpp)
else()
    # Only include header; no .cpp to compile
    message(STATUS "Display disabled: only using displayNotActive.hpp")
endif()

target_include_directories(game_simulation PUBLIC
    ${CMAKE_SOURCE_DIR}/game_simulation
    ${CMAKE_SOURCE_DIR}/players
    ${CMAKE_SOURCE_DIR}
)

# Option to enable the active display implementation.
# Set -DENABLE_DISPLAY=OFF at cmake configure time to disable.
option(ENABLE_DISPLAY "Enable active display (displayActive)" ON)
if(ENABLE_DISPLAY)
  target_compile_definitions(game_simulation PUBLIC DISPLAY)
  # Auto-fetch SFML if not available on the system.
  # Use 2.6.1 release zip to avoid git requirement.
  FetchContent_Declare(
    sfml
    URL https://github.com/SFML/SFML/archive/refs/tags/2.6.1.zip
  )
  set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(SFML_BUILD_DOC OFF CACHE BOOL "" FORCE)
  set(SFML_BUILD_TEST_SUITE OFF CACHE BOOL "" FORCE)
  # Build SFML as static libraries to avoid DLL runtime issues during test discovery
  set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(sfml)

  target_link_libraries(game_simulation PRIVATE sfml-graphics sfml-window sfml-system)
  target_compile_definitions(game_simulation PRIVATE SFML_STATIC)
  message(STATUS "Building with DISPLAY enabled (SFML fetched and linked)")
else()
  message(STATUS "Building with DISPLAY disabled")
endif()
# Enable testing
enable_testing()

# Add subdirectory for tests
add_subdirectory(tests)