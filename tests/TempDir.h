// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "TestEnv.h"

#include <atomic>
#include <filesystem>
#include <fstream>
#include <string>

#if defined(_WIN32)
#include <process.h>
#else
#include <unistd.h>
#endif

namespace fatest {

// RAII temporary directory under the system temp root, unique per process and
// per instance so ctest may run test cases in parallel.
class TempDir {
  public:
    explicit TempDir(const std::string& label) {
        static std::atomic<int> counter{0};
#if defined(_WIN32)
        const int pid = _getpid();
#else
        const int pid = static_cast<int>(getpid());
#endif
        m_path = std::filesystem::temp_directory_path() /
                 ("fa-bridge-test-" + label + "-" + std::to_string(pid) + "-" + std::to_string(++counter));
        std::filesystem::create_directories(m_path);
    }
    ~TempDir() {
        std::error_code ec;
        std::filesystem::remove_all(m_path, ec);
    }
    TempDir(const TempDir&) = delete;
    TempDir& operator=(const TempDir&) = delete;

    const std::filesystem::path& path() const {
        return m_path;
    }

  private:
    std::filesystem::path m_path;
};

// Points FA_BRIDGE_CONFIG_DIR / FA_BRIDGE_CACHE_DIR at fresh temp dirs and
// sets FA_BRIDGE_NO_PROBE for the object's lifetime. Keeps tests hermetic on
// machines that have a real persisted config, registry entry, or FA install.
class HermeticEnv {
  public:
    HermeticEnv() : m_config("cfg"), m_cache("cache") {
        setEnv("FA_BRIDGE_CONFIG_DIR", m_config.path().string().c_str());
        setEnv("FA_BRIDGE_CACHE_DIR", m_cache.path().string().c_str());
        setEnv("FA_BRIDGE_NO_PROBE", "1");
        unsetEnv("FA_INSTALL_DIR");
    }
    ~HermeticEnv() {
        unsetEnv("FA_BRIDGE_CONFIG_DIR");
        unsetEnv("FA_BRIDGE_CACHE_DIR");
        unsetEnv("FA_BRIDGE_NO_PROBE");
        unsetEnv("FA_INSTALL_DIR");
    }
    HermeticEnv(const HermeticEnv&) = delete;
    HermeticEnv& operator=(const HermeticEnv&) = delete;

    const std::filesystem::path& configDir() const {
        return m_config.path();
    }
    const std::filesystem::path& cacheDir() const {
        return m_cache.path();
    }

  private:
    TempDir m_config;
    TempDir m_cache;
};

// Drops an empty file with a .LIB name into dir so it passes install-dir
// validation (discovery checks names only, never contents).
inline void touchLibFile(const std::filesystem::path& dir, const char* name = "FA_1.LIB") {
    std::filesystem::create_directories(dir);
    std::ofstream(dir / name).put('\0');
}

} // namespace fatest
