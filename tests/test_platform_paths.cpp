// SPDX-License-Identifier: GPL-3.0-or-later

#include "PlatformPaths.h"
#include "TestEnv.h"

#include <catch2/catch_test_macros.hpp>

#include <cstdlib>
#include <optional>
#include <string>

using fatest::setEnv;
using fatest::unsetEnv;

namespace {

// Saves and restores an environment variable so path tests can clear HOME/XDG
// bases without disturbing later tests in the same process.
class ScopedEnv {
  public:
    explicit ScopedEnv(const char* name) : m_name(name) {
        if (const char* value = std::getenv(name))
            m_saved = value;
    }
    ~ScopedEnv() {
        if (m_saved)
            setEnv(m_name, m_saved->c_str());
        else
            unsetEnv(m_name);
    }
    ScopedEnv(const ScopedEnv&) = delete;
    ScopedEnv& operator=(const ScopedEnv&) = delete;

  private:
    const char* m_name;
    std::optional<std::string> m_saved;
};

} // namespace

TEST_CASE("PlatformPaths env override wins for config and cache dirs") {
    ScopedEnv savedConfig("FA_BRIDGE_CONFIG_DIR");
    ScopedEnv savedCache("FA_BRIDGE_CACHE_DIR");

    setEnv("FA_BRIDGE_CONFIG_DIR", "/tmp/fa-bridge-forced-config");
    setEnv("FA_BRIDGE_CACHE_DIR", "/tmp/fa-bridge-forced-cache");
    CHECK(fa::configDir() == std::filesystem::path("/tmp/fa-bridge-forced-config"));
    CHECK(fa::cacheDir() == std::filesystem::path("/tmp/fa-bridge-forced-cache"));
}

#if !defined(_WIN32) && !defined(__APPLE__)

TEST_CASE("PlatformPaths resolve under the XDG base dirs on Linux") {
    ScopedEnv savedConfig("FA_BRIDGE_CONFIG_DIR");
    ScopedEnv savedCache("FA_BRIDGE_CACHE_DIR");
    ScopedEnv savedXdgConfig("XDG_CONFIG_HOME");
    ScopedEnv savedXdgCache("XDG_CACHE_HOME");
    ScopedEnv savedHome("HOME");
    unsetEnv("FA_BRIDGE_CONFIG_DIR");
    unsetEnv("FA_BRIDGE_CACHE_DIR");

    SECTION("XDG env bases win") {
        setEnv("XDG_CONFIG_HOME", "/tmp/xdg-config");
        setEnv("XDG_CACHE_HOME", "/tmp/xdg-cache");
        CHECK(fa::configDir() == std::filesystem::path("/tmp/xdg-config/fighters-legacy/fa-bridge"));
        CHECK(fa::cacheDir() == std::filesystem::path("/tmp/xdg-cache/fighters-legacy/fa-bridge"));
    }

    SECTION("HOME fallback applies without XDG") {
        unsetEnv("XDG_CONFIG_HOME");
        unsetEnv("XDG_CACHE_HOME");
        setEnv("HOME", "/tmp/home");
        CHECK(fa::configDir() == std::filesystem::path("/tmp/home/.config/fighters-legacy/fa-bridge"));
        CHECK(fa::cacheDir() == std::filesystem::path("/tmp/home/.cache/fighters-legacy/fa-bridge"));
    }

    SECTION("no base env yields empty paths") {
        unsetEnv("XDG_CONFIG_HOME");
        unsetEnv("XDG_CACHE_HOME");
        unsetEnv("HOME");
        CHECK(fa::configDir().empty());
        CHECK(fa::cacheDir().empty());
    }
}

#endif // Linux-only sections
