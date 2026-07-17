// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "MappedFile.h"

#include <fx/ealib.h>

#include <cstdint>
#include <filesystem>
#include <optional>
#include <vector>

namespace fa {

// One memory-mapped EALIB archive with its parsed directory. LIB archives are
// untrusted user input: open() validates the magic and the entry count itself
// (fx::ealib_read_dir returns an empty vector for both a corrupt directory and
// a genuinely empty archive, so the distinction is made here), and extraction
// is bounds-checked by fx_lib against the mapped size.
class LibArchive {
  public:
    static std::optional<LibArchive> open(std::filesystem::path libPath);

    const std::filesystem::path& path() const {
        return m_path;
    }
    // Source fingerprint inputs for the translation cache.
    uint64_t fileSize() const {
        return m_map.size();
    }
    int64_t mtime() const {
        return m_mtime;
    }
    const std::vector<fx::Entry>& entries() const {
        return m_entries;
    }

    // decompress=true: raw entries verbatim, PKWare DCL entries decompressed;
    // LZSS/PXPK entries return empty and set *unsupported (fx contract).
    std::vector<uint8_t> extract(const fx::Entry& entry, bool decompress = true, bool* unsupported = nullptr) const;

  private:
    std::filesystem::path m_path;
    MappedFile m_map;
    std::vector<fx::Entry> m_entries;
    int64_t m_mtime{0};
};

} // namespace fa
