// SPDX-License-Identifier: GPL-3.0-or-later

// LIB archives are untrusted user input: beyond the happy path these tests
// craft truncated, mislabeled, and out-of-bounds archives and require
// graceful failure, never a crash.

#include "LibArchive.h"
#include "SyntheticLib.h"
#include "TempDir.h"

#include <catch2/catch_test_macros.hpp>

#include <fstream>
#include <vector>

using fatest::TempDir;

TEST_CASE("LibArchive opens a synthetic lib and lists entries with sizes") {
    TempDir tmp("lib");
    const auto file = tmp.path() / "TEST.LIB";
    const std::vector<uint8_t> alpha{1, 2, 3, 4, 5};
    const std::vector<uint8_t> beta{9, 8};
    fatest::writeLib(file, {{"ALPHA.PT", alpha}, {"BETA.SH", beta}});

    auto lib = fa::LibArchive::open(file);
    REQUIRE(lib.has_value());
    REQUIRE(lib->entries().size() == 2);
    CHECK(lib->fileSize() > 0);
    CHECK(lib->extract(lib->entries()[0]) == alpha);
    CHECK(lib->extract(lib->entries()[1]) == beta);
}

TEST_CASE("LibArchive rejects files that are not EALIB archives") {
    TempDir tmp("lib");

    SECTION("wrong magic") {
        const auto file = tmp.path() / "BAD.LIB";
        std::ofstream(file, std::ios::binary) << "NOTEALIB but long enough";
        CHECK_FALSE(fa::LibArchive::open(file).has_value());
    }
    SECTION("shorter than the magic") {
        const auto file = tmp.path() / "TINY.LIB";
        std::ofstream(file, std::ios::binary) << "EA";
        CHECK_FALSE(fa::LibArchive::open(file).has_value());
    }
    SECTION("zero length") {
        const auto file = tmp.path() / "EMPTY.LIB";
        std::ofstream(file, std::ios::binary).flush();
        CHECK_FALSE(fa::LibArchive::open(file).has_value());
    }
    SECTION("missing file") {
        CHECK_FALSE(fa::LibArchive::open(tmp.path() / "GONE.LIB").has_value());
    }
}

TEST_CASE("LibArchive accepts a valid archive with zero entries") {
    TempDir tmp("lib");
    const auto file = tmp.path() / "NIL.LIB";
    fatest::writeLib(file, {});

    auto lib = fa::LibArchive::open(file);
    REQUIRE(lib.has_value());
    CHECK(lib->entries().empty());
}

TEST_CASE("LibArchive rejects a truncated directory") {
    TempDir tmp("lib");
    const auto file = tmp.path() / "CUT.LIB";
    fatest::writeLib(file, {{"ALPHA.PT", {1, 2, 3}}, {"BETA.SH", {4, 5}}});

    // Keep the header (which promises 2 entries) but cut the file inside the
    // directory region.
    std::filesystem::resize_file(file, 7 + 18);
    CHECK_FALSE(fa::LibArchive::open(file).has_value());
}

TEST_CASE("LibArchive handles hostile entries without crashing") {
    TempDir tmp("lib");
    const auto file = tmp.path() / "EVIL.LIB";
    fatest::writeLib(file, {{"ALPHA.PT", {1, 2, 3}}, {"BETA.SH", {4, 5}}});

    SECTION("out-of-bounds entry offset yields empty bytes") {
        fatest::patchEntryOffset(file, 0, 0x7fffff00u);
        const auto lib = fa::LibArchive::open(file);
        // Whether open() rejects the directory or extract() bounds-checks,
        // hostile offsets must never read out of bounds (ASan-verified).
        if (lib.has_value() && !lib->entries().empty())
            CHECK(lib->extract(lib->entries()[0]).empty());
    }

    SECTION("unsupported compression flags yield empty plus the flag") {
        fatest::patchEntryFlags(file, 0, 1); // LZSS, undecodable per fx contract
        const auto lib = fa::LibArchive::open(file);
        REQUIRE(lib.has_value());
        bool unsupported = false;
        CHECK(lib->extract(lib->entries()[0], true, &unsupported).empty());
        CHECK(unsupported);
        // decompress=false returns the stored bytes verbatim instead.
        unsupported = false;
        CHECK(lib->extract(lib->entries()[0], false, &unsupported).size() == 3);
        CHECK_FALSE(unsupported);
    }
}
