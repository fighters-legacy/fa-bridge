cmake_minimum_required(VERSION 3.25)
project(fa-bridge VERSION 0.3.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

option(FA_BUILD_TESTS "Build the fa-bridge test suite" ON)

# fx_lib (fighters-codex, extern/fx_lib) provides the FA format parsers and is
# linked unconditionally — codex ships it cross-platform since its Phase 1.
# The codex root is the embeddable product: added as a subproject it builds
# only lib/ and render/; CLI, GUI, tools, and tests stay top-level-only.
if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/extern/fx_lib/CMakeLists.txt)
    message(FATAL_ERROR
        "extern/fx_lib is missing. Run: git submodule update --init --recursive")
endif()
add_subdirectory(extern/fx_lib)
# fx_lib links into the shared plugin: keep fx symbols out of the plugin's
# export table, and never let upstream warnings trip the warning-as-error
# presets. fx_render builds as part of the embed but is not linked here.
set_target_properties(fx_lib fx_render PROPERTIES
    COMPILE_WARNING_AS_ERROR OFF
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON)

# Bridge source trees are added as the roadmap phases land (docs/roadmap.md):
# bridge/ (Phase 1) exists; transcode/ arrives with Phase 3.
add_subdirectory(bridge)

if(FA_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
