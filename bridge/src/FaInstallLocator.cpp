// SPDX-License-Identifier: GPL-3.0-or-later
#include "FaInstallLocator.h"

#include "AsciiCase.h"
#include "BridgeConfig.h"

#include <cstdlib>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <string>
#endif

namespace fa {

namespace {

#if defined(_WIN32)

// Best-effort registry probe. The exact key/value the FA installer writes is
// not yet documented upstream (fighters-codex tracks it) and is UNVERIFIED
// against a real install — every candidate is validated by isValidInstallDir
// before use, and failure falls through silently to the drive probe/dialog.
std::optional<std::filesystem::path> probeRegistry() {
    static constexpr const char* kSubkeys[] = {
        "SOFTWARE\\Jane's Combat Simulations\\Fighters Anthology",
    };
    static constexpr const char* kValues[] = {"Install Directory", "InstallPath", "PATH", ""};
    static constexpr HKEY kRoots[] = {HKEY_LOCAL_MACHINE, HKEY_CURRENT_USER};
    // FA is a 32-bit app: on x64 its keys live in the WOW6432 view.
    static constexpr DWORD kViews[] = {RRF_RT_REG_SZ, RRF_RT_REG_SZ | RRF_SUBKEY_WOW6432KEY};

    for (HKEY root : kRoots) {
        for (const char* subkey : kSubkeys) {
            for (DWORD view : kViews) {
                for (const char* value : kValues) {
                    char buffer[MAX_PATH]{};
                    DWORD size = sizeof(buffer);
                    if (RegGetValueA(root, subkey, *value ? value : nullptr, view, nullptr, buffer, &size) ==
                            ERROR_SUCCESS &&
                        buffer[0] != '\0') {
                        std::filesystem::path candidate(buffer);
                        if (FaInstallLocator::isValidInstallDir(candidate))
                            return candidate;
                    }
                }
            }
        }
    }
    return std::nullopt;
}

// Probes every fixed drive for the retail default install layout.
std::optional<std::filesystem::path> probeDrives() {
    const DWORD drives = GetLogicalDrives();
    for (char letter = 'A'; letter <= 'Z'; ++letter) {
        if ((drives & (1u << (letter - 'A'))) == 0)
            continue;
        const std::string rootName = std::string(1, letter) + ":\\";
        if (GetDriveTypeA(rootName.c_str()) != DRIVE_FIXED)
            continue;
        std::filesystem::path candidate = rootName + "JANES\\Fighters Anthology";
        if (FaInstallLocator::isValidInstallDir(candidate))
            return candidate;
    }
    return std::nullopt;
}

#endif // _WIN32

} // namespace

bool FaInstallLocator::isValidInstallDir(const std::filesystem::path& dir) {
    std::error_code ec;
    if (!std::filesystem::is_directory(dir, ec))
        return false;
    for (auto it = std::filesystem::directory_iterator(dir, ec); !ec && it != std::filesystem::directory_iterator();
         it.increment(ec)) {
        if (!it->is_regular_file(ec))
            continue;
        if (asciiIEquals(it->path().extension().string(), ".lib"))
            return true;
    }
    return false;
}

std::optional<std::filesystem::path> FaInstallLocator::discover(const std::filesystem::path& configFile,
                                                                bool systemProbes) {
    // 1. Explicit override always wins.
    if (const char* env = std::getenv("FA_INSTALL_DIR"); env != nullptr && *env != '\0') {
        std::filesystem::path candidate(env);
        if (isValidInstallDir(candidate))
            return candidate;
    }

    // 2. The persisted first-run confirmation.
    if (!configFile.empty()) {
        if (auto config = BridgeConfig::load(configFile); config && isValidInstallDir(config->installDir))
            return config->installDir;
    }

    // 3. System probes (Windows registry + default install layout).
    if (const char* noProbe = std::getenv("FA_BRIDGE_NO_PROBE"); noProbe != nullptr && *noProbe != '\0')
        systemProbes = false;
    if (systemProbes) {
#if defined(_WIN32)
        if (auto found = probeRegistry())
            return found;
        if (auto found = probeDrives())
            return found;
#endif
    }

    return std::nullopt;
}

} // namespace fa
