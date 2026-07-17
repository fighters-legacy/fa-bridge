// SPDX-License-Identifier: GPL-3.0-or-later
#include "FaVfs.h"

#include "AsciiCase.h"

#include <algorithm>
#include <utility>

namespace fa {

bool FaVfs::mount(const std::filesystem::path& installDir) {
    clear();

    // Enumerate + case-fold, never literal paths: FA installs vary in case.
    std::vector<std::filesystem::path> libPaths;
    std::error_code ec;
    for (auto it = std::filesystem::directory_iterator(installDir, ec);
         !ec && it != std::filesystem::directory_iterator(); it.increment(ec)) {
        if (!it->is_regular_file(ec))
            continue;
        if (asciiIEquals(it->path().extension().string(), ".lib"))
            libPaths.push_back(it->path());
    }

    // Deterministic registration order (FA's own is OS enumeration order);
    // later mounts shadow earlier duplicates — last registration wins, as in
    // FA's hint index.
    std::sort(libPaths.begin(), libPaths.end(), [](const auto& a, const auto& b) {
        return asciiLower(a.filename().string()) < asciiLower(b.filename().string());
    });

    for (auto& path : libPaths) {
        auto archive = LibArchive::open(std::move(path));
        if (!archive)
            continue; // unparseable archive: skipped, the rest still mount
        m_libs.push_back(std::make_unique<LibArchive>(std::move(*archive)));
        const LibArchive* lib = m_libs.back().get();
        for (const fx::Entry& entry : lib->entries())
            m_byName[asciiLower(entry.name)] = EntryRef{lib, &entry};
    }

    return !m_libs.empty();
}

void FaVfs::clear() {
    m_byName.clear();
    m_libs.clear();
}

std::optional<FaVfs::EntryRef> FaVfs::find(std::string_view fileName) const {
    const auto it = m_byName.find(asciiLower(fileName));
    if (it == m_byName.end())
        return std::nullopt;
    return it->second;
}

std::optional<FaVfs::EntryRef> FaVfs::findStem(std::string_view stem,
                                               std::span<const std::string_view> extensions) const {
    const std::string folded = asciiLower(stem);
    for (const std::string_view ext : extensions) {
        const auto it = m_byName.find(folded + "." + std::string(ext));
        if (it != m_byName.end())
            return it->second;
    }
    return std::nullopt;
}

std::vector<std::string> FaVfs::listStems(std::span<const std::string_view> extensions) const {
    std::vector<std::string> stems;
    for (const auto& [name, ref] : m_byName) {
        const auto dot = name.rfind('.');
        if (dot == std::string::npos)
            continue; // extension-less entries stay findable via find() only
        const std::string_view ext(name.data() + dot + 1, name.size() - dot - 1);
        for (const std::string_view wanted : extensions) {
            if (ext == wanted) {
                stems.emplace_back(name.substr(0, dot));
                break;
            }
        }
    }
    std::sort(stems.begin(), stems.end());
    stems.erase(std::unique(stems.begin(), stems.end()), stems.end());
    return stems;
}

std::vector<uint8_t> FaVfs::read(const EntryRef& ref, bool decompress, bool* unsupported) const {
    return ref.lib->extract(*ref.entry, decompress, unsupported);
}

} // namespace fa
