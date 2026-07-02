cmake_minimum_required(VERSION 3.25)
project(fa-bridge VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# fx_lib does not yet build on Linux/macOS; the default flips ON once
# fighters-codex Phase 1 (cross-platform port) ships. See docs/roadmap.md.
option(FA_WITH_FX_LIB "Build against fx_lib (requires fighters-codex Phase 1)" OFF)

if(FA_WITH_FX_LIB)
    if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/extern/fx_lib/lib/CMakeLists.txt)
        message(FATAL_ERROR
            "extern/fx_lib is missing. Run: git submodule update --init --recursive")
    endif()
    add_subdirectory(extern/fx_lib/lib)
endif()

# Bridge source trees are added here as the roadmap phases land (docs/roadmap.md)
