// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <filesystem>
#include <optional>

namespace fa {

// The plugin's persisted configuration: today a single key, the confirmed FA
// install directory. The file is written as valid TOML so a future switch to a
// real TOML parser reads the same file; reading uses a tolerant single-key
// scanner on purpose — a corrupt or hand-mangled config must degrade to "not
// configured", never to a crash or a refusal to start.
struct BridgeConfig {
    std::filesystem::path installDir;

    // configDir()/config.toml, or an empty path when no config dir resolves.
    static std::filesystem::path defaultPath();

    // Scans for `install_dir = "..."`; ignores comments, blank lines, unknown
    // keys, table headers, and CRLF endings; unescapes \\ and \". Returns
    // nullopt when the file is missing/unreadable or the key is absent, empty,
    // or malformed.
    static std::optional<BridgeConfig> load(const std::filesystem::path& file);

    // Writes valid TOML (basic string; the path is serialized with forward
    // slashes via generic_string()). Creates parent directories. False on any
    // failure.
    bool save(const std::filesystem::path& file) const;
};

} // namespace fa
