# SPDX-License-Identifier: GPL-3.0-or-later

# Catch2 v3 is the unit-test framework (issue #38), fetched at configure time —
# the first configure per build directory needs network access. Matches the
# fighters-codex precedent (same framework, same version).
include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.7.1
    SYSTEM)
FetchContent_MakeAvailable(Catch2)
# Fetched third-party code must never trip the warning-as-error presets.
set_target_properties(Catch2 Catch2WithMain PROPERTIES COMPILE_WARNING_AS_ERROR OFF)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(Catch)

add_executable(fa_bridge_tests
    test_bridge_config.cpp
    test_configure_flow.cpp
    test_fa_content_pack.cpp
    test_fa_vfs.cpp
    test_fx_lib_link.cpp
    test_install_locator.cpp
    test_lib_archive.cpp
    test_mapped_file.cpp
    test_pack_assets.cpp
    test_platform_paths.cpp
    test_real_install.cpp
    test_translation_cache.cpp)
target_link_libraries(fa_bridge_tests PRIVATE fa-bridge-core Catch2::Catch2WithMain)
catch_discover_tests(fa_bridge_tests)

# Loads the built plugin exactly as the engine's ModLoader does. Deliberately a
# plain executable, not a Catch2 target: it is a linear dlopen/LoadLibrary
# rehearsal whose process lifetime mirrors the engine's, and it must stay
# buildable without network access. Paths travel as arguments, not compile
# definitions — Windows paths in compile definitions are a backslash-escape
# hazard.
add_executable(test_plugin_load test_plugin_load.cpp)
target_link_libraries(test_plugin_load PRIVATE fl-headers ${CMAKE_DL_LIBS})
add_dependencies(test_plugin_load fa-bridge)
add_test(NAME plugin_load
         COMMAND test_plugin_load $<TARGET_FILE:fa-bridge>
                 ${CMAKE_BINARY_DIR}/stage/mods/fa-bridge/manifest.toml)
