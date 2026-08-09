cmake_minimum_required(VERSION 3.14)
project(CatanAPI)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
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

# ----------------------------
# 🔗 Self-contained executables on MinGW
# ----------------------------
# A MinGW toolchain (llvm-mingw, WinLibs, MSYS2) links its C++ runtime as DLLs
# that live in the toolchain's own bin/ directory: libc++.dll or
# libstdc++-6.dll, plus libunwind.dll and libwinpthread-1.dll. Nothing puts
# those next to the build output, so launching a test binary or run.exe from
# Explorer, from ctest, or from any shell without the toolchain on PATH fails
# with a "libc++.dll was not found" loader dialog rather than a useful error.
#
# Linking them statically costs a little binary size and makes the executables
# runnable anywhere. MSVC, Linux, and macOS are unaffected.
#
# Must come after the ASAN block: the sanitizer runtime cannot be linked
# statically here, so -static and -fsanitize=address are mutually exclusive.
if (MINGW AND NOT ENABLE_ASAN)
  add_link_options(-static)
endif()

include(FetchContent)

# ----------------------------
# 🧩 GoogleTest setup (needed for tests)
# ----------------------------
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
  # Without this, CMake >= 3.24 warns (CMP0135) and reuses the archive's own
  # timestamps, so a changed URL can fail to trigger a rebuild.
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
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
# 🎮 Game simulation library
# ----------------------------
add_subdirectory(game_simulation)

# ----------------------------
# 🎮 Players library
# ----------------------------
add_subdirectory(players)

# ----------------------------
# 🧪 Enable testing
# ----------------------------
enable_testing()
add_subdirectory(tests)

# ----------------------------
# ▶️ Add runs subdirectory
# ----------------------------
add_subdirectory(runs)
