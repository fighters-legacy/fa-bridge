// SPDX-License-Identifier: GPL-3.0-or-later

#include "FaVfs.h"
#include "SyntheticLib.h"
#include "TempDir.h"

#include <catch2/catch_test_macros.hpp>

#include <fstream>
#include <string_view>
#include <vector>

using namespace std::string_view_literals;
using fatest::TempDir;
using fatest::writeLib;

namespace {
constexpr std::string_view kPt[] = {"pt"sv};
constexpr std::string_view kPicThenRaw[] = {"pic"sv, "raw"sv};
} // namespace

TEST_CASE("FaVfs mounts every lib in a directory case insensitively") {
    TempDir tmp("vfs");
    writeLib(tmp.path() / "FA_1.LIB", {{"A.PT", {1}}});
    writeLib(tmp.path() / "fa_4c.lib", {{"B.PT", {2}}});
    writeLib(tmp.path() / "Fa_7.LiB", {{"C.PT", {3}}});
    std::ofstream(tmp.path() / "README.TXT") << "ignored";

    fa::FaVfs vfs;
    REQUIRE(vfs.mount(tmp.path()));
    CHECK(vfs.libCount() == 3);
    CHECK(vfs.entryCount() == 3);
}

TEST_CASE("FaVfs mount fails on a directory with no usable libs") {
    TempDir tmp("vfs");
    fa::FaVfs vfs;

    SECTION("empty directory") {
        CHECK_FALSE(vfs.mount(tmp.path()));
    }
    SECTION("only an unparseable lib") {
        std::ofstream(tmp.path() / "FAKE.LIB") << "not an archive";
        CHECK_FALSE(vfs.mount(tmp.path()));
    }
}

TEST_CASE("FaVfs skips an unparseable lib and mounts the rest") {
    TempDir tmp("vfs");
    writeLib(tmp.path() / "GOOD.LIB", {{"A.PT", {1}}});
    std::ofstream(tmp.path() / "BAD.LIB") << "garbage";

    fa::FaVfs vfs;
    REQUIRE(vfs.mount(tmp.path()));
    CHECK(vfs.libCount() == 1);
    CHECK(vfs.find("a.pt").has_value());
}

TEST_CASE("FaVfs find matches any case") {
    TempDir tmp("vfs");
    writeLib(tmp.path() / "FA_1.LIB", {{"F22.PT", {1, 2}}});

    fa::FaVfs vfs;
    REQUIRE(vfs.mount(tmp.path()));
    CHECK(vfs.find("F22.PT").has_value());
    CHECK(vfs.find("f22.pt").has_value());
    CHECK(vfs.find("F22.pt").has_value());
    CHECK_FALSE(vfs.find("f23.pt").has_value());
}

TEST_CASE("FaVfs later lib shadows duplicate names from earlier lib") {
    TempDir tmp("vfs");
    // Case-folded order is a.lib < B.LIB regardless of the on-disk case, so
    // B.LIB registers last and must win — FA's last-registration-wins rule
    // over our deterministic mount order.
    writeLib(tmp.path() / "a.lib", {{"SAME.PT", {0xAA}}});
    writeLib(tmp.path() / "B.LIB", {{"SAME.PT", {0xBB}}});

    fa::FaVfs vfs;
    REQUIRE(vfs.mount(tmp.path()));
    const auto ref = vfs.find("same.pt");
    REQUIRE(ref.has_value());
    CHECK(vfs.read(*ref) == std::vector<uint8_t>{0xBB});
}

TEST_CASE("FaVfs findStem honors extension precedence order") {
    TempDir tmp("vfs");
    writeLib(tmp.path() / "FA_1.LIB", {{"SKIN.PIC", {0x11}}, {"SKIN.RAW", {0x22}}});

    fa::FaVfs vfs;
    REQUIRE(vfs.mount(tmp.path()));
    const auto ref = vfs.findStem("skin", kPicThenRaw);
    REQUIRE(ref.has_value());
    CHECK(vfs.read(*ref) == std::vector<uint8_t>{0x11}); // pic listed first, pic wins
}

TEST_CASE("FaVfs listStems dedupes across libs and extensions") {
    TempDir tmp("vfs");
    writeLib(tmp.path() / "FA_1.LIB", {{"SKIN.PIC", {1}}, {"MENU.PIC", {2}}});
    writeLib(tmp.path() / "FA_2.LIB", {{"SKIN.RAW", {3}}, {"&LOOP.PIC", {4}}});

    fa::FaVfs vfs;
    REQUIRE(vfs.mount(tmp.path()));
    const auto stems = vfs.listStems(kPicThenRaw);
    REQUIRE(stems.size() == 3);
    // Sorted, lowercase, prefix characters preserved (naming policy is a
    // Phase 3 transcoder decision — the VFS reports what exists).
    CHECK(stems[0] == "&loop");
    CHECK(stems[1] == "menu");
    CHECK(stems[2] == "skin");
}

TEST_CASE("FaVfs extension-less entries are findable but never listed") {
    TempDir tmp("vfs");
    writeLib(tmp.path() / "FA_1.LIB", {{"README", {1}}, {"A.PT", {2}}});

    fa::FaVfs vfs;
    REQUIRE(vfs.mount(tmp.path()));
    CHECK(vfs.find("readme").has_value());
    CHECK(vfs.listStems(kPt) == std::vector<std::string>{"a"});
}

TEST_CASE("FaVfs queries on an unmounted vfs return empty") {
    fa::FaVfs vfs;
    CHECK(vfs.libCount() == 0);
    CHECK_FALSE(vfs.find("a.pt").has_value());
    CHECK_FALSE(vfs.findStem("a", kPt).has_value());
    CHECK(vfs.listStems(kPt).empty());
}

TEST_CASE("FaVfs mount replaces any previous state") {
    TempDir first("vfs-first");
    TempDir second("vfs-second");
    writeLib(first.path() / "FA_1.LIB", {{"OLD.PT", {1}}});
    writeLib(second.path() / "FA_2.LIB", {{"NEW.PT", {2}}});

    fa::FaVfs vfs;
    REQUIRE(vfs.mount(first.path()));
    REQUIRE(vfs.mount(second.path()));
    CHECK(vfs.libCount() == 1);
    CHECK_FALSE(vfs.find("old.pt").has_value());
    CHECK(vfs.find("new.pt").has_value());

    // A failed mount also clears — no stale entries survive.
    TempDir empty("vfs-empty");
    CHECK_FALSE(vfs.mount(empty.path()));
    CHECK(vfs.libCount() == 0);
    CHECK_FALSE(vfs.find("new.pt").has_value());
}
