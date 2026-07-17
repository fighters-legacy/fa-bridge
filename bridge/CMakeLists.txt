# SPDX-License-Identifier: GPL-3.0-or-later

# Engine interface headers, vendored verbatim with a recorded pin
# (extern/fl-headers/PIN.md).
add_library(fl-headers INTERFACE)
target_include_directories(fl-headers INTERFACE ${CMAKE_SOURCE_DIR}/extern/fl-headers)

configure_file(src/FaBridgeVersion.h.in ${CMAKE_CURRENT_BINARY_DIR}/generated/FaBridgeVersion.h @ONLY)

# Static core keeps the pack unit-testable without loading the shared library;
# the Phase 2/3 discovery and transcode code lands here too.
add_library(fa-bridge-core STATIC
    src/BridgeConfig.cpp
    src/FaAssetTypes.cpp
    src/FaContentPack.cpp
    src/FaInstallLocator.cpp
    src/FaVfs.cpp
    src/LibArchive.cpp
    src/MappedFile.cpp
    src/PlatformPaths.cpp
    src/TranslationCache.cpp)
target_include_directories(fa-bridge-core PUBLIC src ${CMAKE_CURRENT_BINARY_DIR}/generated)
# fx::lib is PUBLIC: the mount layer's headers expose fx::Entry, and the test
# suite builds synthetic archives with fx::ealib_build.
target_link_libraries(fa-bridge-core PUBLIC fl-headers fx::lib)
# Hidden visibility here too: the core's objects are linked into the plugin,
# and only the factory (marked FA_BRIDGE_EXPORT) may be visible outside it.
set_target_properties(fa-bridge-core PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON)
if(MSVC)
    # std::getenv in FaContentPack::init() trips C4996 under /W4 + /WX.
    target_compile_definitions(fa-bridge-core PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

# SHARED, not MODULE, deliberately: the engine's ModLoader derives the plugin
# filename from the manifest id (libfa-bridge.so / fa-bridge.dll /
# libfa-bridge.dylib), and a MODULE library would get a .so suffix on macOS.
add_library(fa-bridge SHARED src/plugin_entry.cpp)
target_link_libraries(fa-bridge PRIVATE fa-bridge-core)
set_target_properties(fa-bridge PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON)

# Fail unresolved symbols at our link instead of inside the engine's
# dlopen(RTLD_NOW). Sanitizer builds must skip this: their runtime symbols
# stay undefined in shared libraries by design.
if(UNIX AND NOT APPLE AND NOT CMAKE_CXX_FLAGS MATCHES "sanitize")
    target_link_options(fa-bridge PRIVATE LINKER:--no-undefined)
endif()

# Stage a drop-in mod directory: copy stage/mods/fa-bridge into an engine's
# mods/ directory to test against a real ModLoader (docs/development.md).
set(FA_BRIDGE_STAGE_DIR ${CMAKE_BINARY_DIR}/stage/mods/fa-bridge)
configure_file(${CMAKE_SOURCE_DIR}/packaging/manifest.toml.in ${FA_BRIDGE_STAGE_DIR}/manifest.toml @ONLY)
add_custom_command(TARGET fa-bridge POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:fa-bridge> ${FA_BRIDGE_STAGE_DIR}
    COMMENT "Staging mod directory: ${FA_BRIDGE_STAGE_DIR}")
