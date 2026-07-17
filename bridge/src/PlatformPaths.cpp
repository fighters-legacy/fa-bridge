// SPDX-License-Identifier: GPL-3.0-or-later
#include "PlatformPaths.h"

#include <cstdlib>

namespace fa {

namespace {

std::filesystem::path fromEnv(const char* name) {
    const char* value = std::getenv(name);
    if (value != nullptr && *value != '\0')
        return std::filesystem::path(value);
    return {};
}

std::filesystem::path withSuffix(std::filesystem::path base) {
    if (base.empty())
        return {};
    return base / "fighters-legacy" / "fa-bridge";
}

} // namespace

std::filesystem::path configDir() {
    if (auto forced = fromEnv("FA_BRIDGE_CONFIG_DIR"); !forced.empty())
        return forced;
#if defined(_WIN32)
    return withSuffix(fromEnv("APPDATA"));
#elif defined(__APPLE__)
    if (auto home = fromEnv("HOME"); !home.empty())
        return withSuffix(home / "Library" / "Application Support");
    return {};
#else
    if (auto xdg = fromEnv("XDG_CONFIG_HOME"); !xdg.empty())
        return withSuffix(xdg);
    if (auto home = fromEnv("HOME"); !home.empty())
        return withSuffix(home / ".config");
    return {};
#endif
}

std::filesystem::path cacheDir() {
    if (auto forced = fromEnv("FA_BRIDGE_CACHE_DIR"); !forced.empty())
        return forced;
#if defined(_WIN32)
    return withSuffix(fromEnv("LOCALAPPDATA"));
#elif defined(__APPLE__)
    if (auto home = fromEnv("HOME"); !home.empty())
        return withSuffix(home / "Library" / "Caches");
    return {};
#else
    if (auto xdg = fromEnv("XDG_CACHE_HOME"); !xdg.empty())
        return withSuffix(xdg);
    if (auto home = fromEnv("HOME"); !home.empty())
        return withSuffix(home / ".cache");
    return {};
#endif
}

} // namespace fa
