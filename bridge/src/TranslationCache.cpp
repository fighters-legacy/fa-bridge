// SPDX-License-Identifier: GPL-3.0-or-later
#include "TranslationCache.h"

#include "AsciiCase.h"

#include <fx/ealib.h>

#include <fstream>
#include <utility>

namespace fa {

TranslationCache::TranslationCache(std::filesystem::path root) : m_root(std::move(root)) {
    if (m_root.empty())
        return;
    std::error_code ec;
    std::filesystem::create_directories(m_root, ec);
    m_enabled = !ec && std::filesystem::is_directory(m_root, ec);
}

std::filesystem::path TranslationCache::pathFor(const Key& key) const {
    // ealib_safe_name maps FA's prefix characters ('&' looping audio) and any
    // hostile path characters out of archive-derived names.
    return m_root / key.stage /
           ("v" + std::to_string(kSchemaVersion) + "-" + fx::ealib_safe_name(key.assetName.c_str()) + "-" +
            std::to_string(key.sourceSize) + "-" + std::to_string(key.sourceMtime) + ".bin");
}

std::optional<std::vector<uint8_t>> TranslationCache::get(const Key& key) const {
    if (!m_enabled)
        return std::nullopt;
    std::ifstream in(pathFor(key), std::ios::binary | std::ios::ate);
    if (!in)
        return std::nullopt;
    const std::streamsize size = in.tellg();
    if (size < 0)
        return std::nullopt;
    std::vector<uint8_t> bytes(static_cast<size_t>(size));
    in.seekg(0);
    if (size > 0 && !in.read(reinterpret_cast<char*>(bytes.data()), size))
        return std::nullopt;
    return bytes;
}

bool TranslationCache::put(const Key& key, std::span<const uint8_t> bytes) {
    if (!m_enabled)
        return false;
    const auto target = pathFor(key);

    std::error_code ec;
    std::filesystem::create_directories(target.parent_path(), ec);
    if (ec)
        return false;

    // Write-temp-then-rename so a torn write can never surface as a hit.
    const auto temp = target.string() + ".tmp";
    {
        std::ofstream out(temp, std::ios::binary | std::ios::trunc);
        if (!out || !out.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size())))
            return false;
    }
    std::filesystem::rename(temp, target, ec);
    if (ec) {
        std::filesystem::remove(temp, ec);
        return false;
    }
    return true;
}

std::vector<uint8_t> readWithCache(const FaVfs& vfs, TranslationCache& cache, const FaVfs::EntryRef& ref,
                                   bool* unsupported) {
    if (ref.entry->flags != 4 || !cache.enabled())
        return vfs.read(ref, true, unsupported);

    const TranslationCache::Key key{"extract", asciiLower(ref.entry->name), ref.lib->fileSize(), ref.lib->mtime()};
    if (auto hit = cache.get(key)) {
        if (unsupported != nullptr)
            *unsupported = false;
        return std::move(*hit);
    }

    auto bytes = vfs.read(ref, true, unsupported);
    if (!bytes.empty())
        cache.put(key, bytes);
    return bytes;
}

} // namespace fa
