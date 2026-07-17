// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

// Synthetic EALIB fixtures, generated at test runtime via fx::ealib_build —
// no FA assets are ever committed (temp .LIB files land in build/system temp
// dirs, which .gitignore blocks anyway).

#include <fx/ealib.h>

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <utility>
#include <vector>

namespace fatest {

using LibFiles = std::vector<std::pair<std::string, std::vector<uint8_t>>>;

inline void writeLib(const std::filesystem::path& file, const LibFiles& files) {
    const auto bytes = fx::ealib_build(files);
    std::ofstream out(file, std::ios::binary);
    out.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
}

// Drops a minimal VALID archive named like a stock FA lib into dir — the
// mount-aware replacement for touchLibFile wherever init()/configure() runs.
inline void writeStockLib(const std::filesystem::path& dir, const char* name = "FA_1.LIB") {
    std::filesystem::create_directories(dir);
    writeLib(dir / name, {{"F22.PT", {0x01, 0x02, 0x03}}});
}

// Overwrites the flags byte of directory entry i. Layout per LIB.md: "EALIB"
// magic (5) + uint16 count (2), then 18-byte entries of 13-byte name + flags
// + uint32 offset — so entry i's flags byte sits at 7 + 18*i + 13.
inline void patchEntryFlags(const std::filesystem::path& file, size_t entryIndex, uint8_t flags) {
    std::fstream io(file, std::ios::binary | std::ios::in | std::ios::out);
    io.seekp(static_cast<std::streamoff>(7 + entryIndex * 18 + 13));
    io.put(static_cast<char>(flags));
}

// Overwrites entry i's 32-bit offset field (little-endian) — for crafting
// out-of-bounds directory entries.
inline void patchEntryOffset(const std::filesystem::path& file, size_t entryIndex, uint32_t offset) {
    std::fstream io(file, std::ios::binary | std::ios::in | std::ios::out);
    io.seekp(static_cast<std::streamoff>(7 + entryIndex * 18 + 14));
    const char bytes[4] = {static_cast<char>(offset & 0xff), static_cast<char>((offset >> 8) & 0xff),
                           static_cast<char>((offset >> 16) & 0xff), static_cast<char>((offset >> 24) & 0xff)};
    io.write(bytes, 4);
}

} // namespace fatest
