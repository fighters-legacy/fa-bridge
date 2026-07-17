// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>

namespace fa {

// Read-only whole-file memory map (POSIX mmap / Win32 MapViewOfFile),
// move-only RAII. Exists because a full FA install carries ~1 GB of .LIB
// archives and fx_lib's ealib API is buffer-based: mapping keeps the archives
// out of heap memory entirely — only directories and extracted entries ever
// materialize. Zero-length files are valid: data()==nullptr, size()==0, no
// syscall is made (mmap and CreateFileMapping both reject length 0).
class MappedFile {
  public:
    static std::optional<MappedFile> open(const std::filesystem::path& path);

    MappedFile() = default;
    MappedFile(MappedFile&& other) noexcept;
    MappedFile& operator=(MappedFile&& other) noexcept;
    MappedFile(const MappedFile&) = delete;
    MappedFile& operator=(const MappedFile&) = delete;
    ~MappedFile();

    const uint8_t* data() const {
        return m_data;
    }
    size_t size() const {
        return m_size;
    }

  private:
    const uint8_t* m_data{nullptr};
    size_t m_size{0};
};

} // namespace fa
