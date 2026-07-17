// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "FaVfs.h"

#include <cstdint>
#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace fa {

// Persistent per-user byte cache under PlatformPaths::cacheDir() (issue #13).
// Self-invalidating: the file name embeds the schema version and the source
// fingerprint (containing-LIB size + mtime), so a changed source simply
// misses; stale files are ignored (pruning is deliberately future work — the
// contents are regenerable and kSchemaVersion bumps force a refresh). There
// is no checksum: torn writes are prevented by write-temp-then-rename. An
// unusable root degrades to disabled(): get/put become no-ops and the bridge
// works without the cache.
class TranslationCache {
  public:
    static constexpr int kSchemaVersion = 1;

    struct Key {
        std::string stage;     // "extract" today; "png"/"glb"/"ogg"/"toml" arrive with Phase 3
        std::string assetName; // lowercase source entry name, e.g. "f22.pt"
        uint64_t sourceSize;   // containing LIB file size
        int64_t sourceMtime;   // containing LIB mtime (filesystem clock ticks)
    };

    TranslationCache() = default; // disabled
    explicit TranslationCache(std::filesystem::path root);

    bool enabled() const {
        return m_enabled;
    }
    std::optional<std::vector<uint8_t>> get(const Key& key) const;
    bool put(const Key& key, std::span<const uint8_t> bytes);

  private:
    std::filesystem::path pathFor(const Key& key) const;

    std::filesystem::path m_root;
    bool m_enabled{false};
};

// The shared cached-read path the Phase 3 transcoders build on: DCL-compressed
// entries (flags 4) are cached decompressed under stage "extract"; raw and
// unsupported entries read straight off the map (caching bytes that are
// already zero-copy would be a pessimization). Extraction failures are never
// cached.
std::vector<uint8_t> readWithCache(const FaVfs& vfs, TranslationCache& cache, const FaVfs::EntryRef& ref,
                                   bool* unsupported = nullptr);

} // namespace fa
