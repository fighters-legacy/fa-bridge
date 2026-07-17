// SPDX-License-Identifier: GPL-3.0-or-later
#include "LibArchive.h"

#include <cstring>
#include <utility>

namespace fa {

std::optional<LibArchive> LibArchive::open(std::filesystem::path libPath) {
    auto mapped = MappedFile::open(libPath);
    if (!mapped)
        return std::nullopt;

    // Header: "EALIB" magic + uint16 little-endian count.
    static constexpr char kMagic[] = {'E', 'A', 'L', 'I', 'B'};
    static constexpr size_t kHeaderSize = sizeof(kMagic) + 2;
    if (mapped->size() < kHeaderSize || std::memcmp(mapped->data(), kMagic, sizeof(kMagic)) != 0)
        return std::nullopt;
    const uint16_t count = static_cast<uint16_t>(mapped->data()[5] | (static_cast<uint16_t>(mapped->data()[6]) << 8));

    auto entries = fx::ealib_read_dir(mapped->data(), mapped->size());
    // read_dir returns empty for corrupt directories AND for valid empty
    // archives; the header count disambiguates (truncated directory, count
    // promising more entries than the file holds, etc. all fail here).
    if (entries.size() != count)
        return std::nullopt;

    std::error_code ec;
    const auto stamp = std::filesystem::last_write_time(libPath, ec);

    LibArchive archive;
    archive.m_path = std::move(libPath);
    archive.m_map = std::move(*mapped);
    archive.m_entries = std::move(entries);
    archive.m_mtime = ec ? 0 : static_cast<int64_t>(stamp.time_since_epoch().count());
    return archive;
}

std::vector<uint8_t> LibArchive::extract(const fx::Entry& entry, bool decompress, bool* unsupported) const {
    return fx::ealib_extract(m_map.data(), m_map.size(), entry, decompress, unsupported);
}

} // namespace fa
