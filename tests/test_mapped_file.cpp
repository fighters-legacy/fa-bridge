// SPDX-License-Identifier: GPL-3.0-or-later

#include "MappedFile.h"
#include "TempDir.h"

#include <catch2/catch_test_macros.hpp>

#include <cstring>
#include <fstream>
#include <utility>

using fatest::TempDir;

TEST_CASE("MappedFile maps an existing file and exposes its bytes") {
    TempDir tmp("map");
    const auto file = tmp.path() / "payload.bin";
    std::ofstream(file, std::ios::binary) << "hello mapped world";

    auto mapped = fa::MappedFile::open(file);
    REQUIRE(mapped.has_value());
    REQUIRE(mapped->size() == 18);
    CHECK(std::memcmp(mapped->data(), "hello mapped world", 18) == 0);
}

TEST_CASE("MappedFile open fails for a missing file and for a directory") {
    TempDir tmp("map");
    CHECK_FALSE(fa::MappedFile::open(tmp.path() / "missing.bin").has_value());
    CHECK_FALSE(fa::MappedFile::open(tmp.path()).has_value());
}

TEST_CASE("MappedFile zero length file maps as empty without a syscall") {
    TempDir tmp("map");
    const auto file = tmp.path() / "empty.bin";
    std::ofstream(file, std::ios::binary).flush();

    auto mapped = fa::MappedFile::open(file);
    REQUIRE(mapped.has_value());
    CHECK(mapped->data() == nullptr);
    CHECK(mapped->size() == 0);
}

TEST_CASE("MappedFile move transfers ownership without double unmap") {
    TempDir tmp("map");
    const auto file = tmp.path() / "payload.bin";
    std::ofstream(file, std::ios::binary) << "abc";

    SECTION("move construct") {
        auto mapped = fa::MappedFile::open(file);
        REQUIRE(mapped.has_value());
        fa::MappedFile moved(std::move(*mapped));
        CHECK(moved.size() == 3);
        CHECK(mapped->data() == nullptr); // NOLINT: moved-from state is defined here
    }

    SECTION("move assign") {
        auto a = fa::MappedFile::open(file);
        auto b = fa::MappedFile::open(file);
        REQUIRE(a.has_value());
        REQUIRE(b.has_value());
        *a = std::move(*b); // a's original mapping is released here
        CHECK(a->size() == 3);
        CHECK(b->data() == nullptr); // NOLINT: moved-from state is defined here
    }
    // The ASan leg proves no leak/double-unmap on either path.
}
