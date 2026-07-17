// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <filesystem>

namespace fa {

// Per-OS user directories for the bridge, ending in "fighters-legacy/fa-bridge".
// Resolution: the env overrides FA_BRIDGE_CONFIG_DIR / FA_BRIDGE_CACHE_DIR win
// unconditionally (documented dev/test knobs — they keep tests hermetic and let
// a user relocate the state); otherwise the platform convention:
//   config — $XDG_CONFIG_HOME|$HOME/.config (Linux), %APPDATA% (Windows),
//            $HOME/Library/Application Support (macOS)
//   cache  — $XDG_CACHE_HOME|$HOME/.cache (Linux), %LOCALAPPDATA% (Windows),
//            $HOME/Library/Caches (macOS)
// Returns an empty path when no base can be determined (e.g. HOME unset) —
// callers treat that as "persistence unavailable" and degrade gracefully.
// Windows note: bases come from narrow getenv (ANSI codepage); non-ASCII
// profile paths are a documented limitation (FA-era data is 8.3 ASCII).
std::filesystem::path configDir();
std::filesystem::path cacheDir();

} // namespace fa
