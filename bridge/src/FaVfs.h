// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "LibArchive.h"

#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace fa {

// One flat case-insensitive namespace over every .LIB in the install
// directory — the same shape FA itself builds at startup (LibStartUp's hint
// index; fighters-codex docs/fa/memory-resource.md): duplicate names resolve
// to the LAST registration. FA registers in OS directory-enumeration order;
// the bridge mounts in case-folded filename order instead so the result is
// deterministic across platforms and filesystems. FA's loose-file fallback
// layer (extension-routed subdirectories) is not mounted — a later phase if
// real installs need it.
class FaVfs {
  public:
    struct EntryRef {
        const LibArchive* lib;
        const fx::Entry* entry;
    };

    // Mounts every *.LIB (matched case-insensitively) directly under
    // installDir. Unparseable archives are skipped. Replaces any previous
    // mount state. False when zero archives mount.
    bool mount(const std::filesystem::path& installDir);
    void clear();

    size_t libCount() const {
        return m_libs.size();
    }
    size_t entryCount() const {
        return m_byName.size();
    }

    // Full entry name ("f22.pt"), any case.
    std::optional<EntryRef> find(std::string_view fileName) const;
    // First extension in `extensions` (lookup precedence) that has `stem`.
    std::optional<EntryRef> findStem(std::string_view stem, std::span<const std::string_view> extensions) const;
    // Lowercase stems of every entry whose extension is in `extensions`,
    // sorted and deduplicated.
    std::vector<std::string> listStems(std::span<const std::string_view> extensions) const;

    std::vector<uint8_t> read(const EntryRef& ref, bool decompress = true, bool* unsupported = nullptr) const;

  private:
    std::vector<std::unique_ptr<LibArchive>> m_libs;    // stable addresses for EntryRef
    std::unordered_map<std::string, EntryRef> m_byName; // key: lowercase "name.ext"
};

} // namespace fa
