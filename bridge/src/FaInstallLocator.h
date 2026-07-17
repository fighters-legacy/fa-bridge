// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <filesystem>
#include <optional>

namespace fa {

// Locates the user's Fighters Anthology installation. All file-name matching
// is case-insensitive by enumeration (never a literal-path existence check) —
// FA installs vary in filename case and Linux filesystems are case-sensitive.
class FaInstallLocator {
  public:
    // True when dir is a directory containing at least one "*.LIB" file,
    // matched case-insensitively.
    static bool isValidInstallDir(const std::filesystem::path& dir);

    // Discovery chain, first valid candidate wins:
    //   1. FA_INSTALL_DIR env var (explicit override)
    //   2. the persisted config file (configFile; skipped when empty)
    //   3. Windows only: registry best-effort, then a fixed-drive probe of
    //      <X>:\JANES\Fighters Anthology
    // Invalid candidates fall through silently. System probes (step 3) are
    // skipped when systemProbes is false or the FA_BRIDGE_NO_PROBE env var is
    // set (dev/test knob).
    static std::optional<std::filesystem::path> discover(const std::filesystem::path& configFile,
                                                         bool systemProbes = true);
};

} // namespace fa
