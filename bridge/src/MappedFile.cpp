// SPDX-License-Identifier: GPL-3.0-or-later
#include "MappedFile.h"

#include <utility>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace fa {

std::optional<MappedFile> MappedFile::open(const std::filesystem::path& path) {
    std::error_code ec;
    if (!std::filesystem::is_regular_file(path, ec))
        return std::nullopt;

#if defined(_WIN32)
    // Native wide-character API via path::c_str() — no codepage loss.
    HANDLE file = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE)
        return std::nullopt;

    LARGE_INTEGER size{};
    if (!GetFileSizeEx(file, &size) || size.QuadPart < 0) {
        CloseHandle(file);
        return std::nullopt;
    }
    if (size.QuadPart == 0) {
        CloseHandle(file);
        return MappedFile{}; // valid empty mapping
    }

    HANDLE mapping = CreateFileMappingW(file, nullptr, PAGE_READONLY, 0, 0, nullptr);
    CloseHandle(file); // the mapping object keeps the file open
    if (mapping == nullptr)
        return std::nullopt;

    void* view = MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, 0);
    CloseHandle(mapping); // the view keeps the mapping object alive
    if (view == nullptr)
        return std::nullopt;

    MappedFile mapped;
    mapped.m_data = static_cast<const uint8_t*>(view);
    mapped.m_size = static_cast<size_t>(size.QuadPart);
    return mapped;
#else
    const int fd = ::open(path.c_str(), O_RDONLY);
    if (fd < 0)
        return std::nullopt;

    // "= {}" rather than "{}": clang-format versions disagree on brace-init
    // after an elaborated type specifier, and the CI format check must pass on
    // both the pinned local version and the runner's.
    struct stat st = {};
    if (::fstat(fd, &st) != 0 || !S_ISREG(st.st_mode)) {
        ::close(fd);
        return std::nullopt;
    }
    if (st.st_size == 0) {
        ::close(fd);
        return MappedFile{}; // valid empty mapping
    }

    void* view = ::mmap(nullptr, static_cast<size_t>(st.st_size), PROT_READ, MAP_PRIVATE, fd, 0);
    ::close(fd); // the mapping persists past the descriptor
    if (view == MAP_FAILED)
        return std::nullopt;

    MappedFile mapped;
    mapped.m_data = static_cast<const uint8_t*>(view);
    mapped.m_size = static_cast<size_t>(st.st_size);
    return mapped;
#endif
}

MappedFile::MappedFile(MappedFile&& other) noexcept
    : m_data(std::exchange(other.m_data, nullptr)), m_size(std::exchange(other.m_size, 0)) {}

MappedFile& MappedFile::operator=(MappedFile&& other) noexcept {
    if (this != &other) {
        this->~MappedFile();
        m_data = std::exchange(other.m_data, nullptr);
        m_size = std::exchange(other.m_size, 0);
    }
    return *this;
}

MappedFile::~MappedFile() {
    if (m_data != nullptr) {
#if defined(_WIN32)
        UnmapViewOfFile(m_data);
#else
        ::munmap(const_cast<uint8_t*>(m_data), m_size);
#endif
        m_data = nullptr;
        m_size = 0;
    }
}

} // namespace fa
