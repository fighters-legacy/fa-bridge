// SPDX-License-Identifier: GPL-3.0-or-later

#include "FaVfs.h"
#include "SyntheticLib.h"
#include "TempDir.h"
#include "TranslationCache.h"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <string>
#include <vector>

#if !defined(_WIN32)
#include <sys/stat.h>
#include <unistd.h>
#endif

using fatest::TempDir;

namespace {

fa::TranslationCache::Key keyFor(const std::string& name, uint64_t size = 1000, int64_t mtime = 42) {
    return fa::TranslationCache::Key{"extract", name, size, mtime};
}

const std::vector<uint8_t> kBytes{0xDE, 0xAD, 0xBE, 0xEF};

} // namespace

TEST_CASE("TranslationCache put then get round trips bytes") {
    TempDir tmp("cache");
    fa::TranslationCache cache(tmp.path());
    REQUIRE(cache.enabled());

    REQUIRE(cache.put(keyFor("f22.pt"), kBytes));
    const auto hit = cache.get(keyFor("f22.pt"));
    REQUIRE(hit.has_value());
    CHECK(*hit == kBytes);
}

TEST_CASE("TranslationCache misses on any fingerprint change") {
    TempDir tmp("cache");
    fa::TranslationCache cache(tmp.path());
    REQUIRE(cache.put(keyFor("f22.pt", 1000, 42), kBytes));

    SECTION("different source size") {
        CHECK_FALSE(cache.get(keyFor("f22.pt", 1001, 42)).has_value());
    }
    SECTION("different source mtime") {
        CHECK_FALSE(cache.get(keyFor("f22.pt", 1000, 43)).has_value());
    }
    SECTION("different asset name") {
        CHECK_FALSE(cache.get(keyFor("f23.pt", 1000, 42)).has_value());
    }
    SECTION("different stage") {
        auto key = keyFor("f22.pt");
        key.stage = "glb";
        CHECK_FALSE(cache.get(key).has_value());
    }
}

TEST_CASE("TranslationCache distinct stages store separately") {
    TempDir tmp("cache");
    fa::TranslationCache cache(tmp.path());
    auto extractKey = keyFor("f22.pt");
    auto glbKey = keyFor("f22.pt");
    glbKey.stage = "glb";
    const std::vector<uint8_t> other{0x01};

    REQUIRE(cache.put(extractKey, kBytes));
    REQUIRE(cache.put(glbKey, other));
    CHECK(*cache.get(extractKey) == kBytes);
    CHECK(*cache.get(glbKey) == other);
}

TEST_CASE("TranslationCache put overwrites an existing entry") {
    TempDir tmp("cache");
    fa::TranslationCache cache(tmp.path());
    const std::vector<uint8_t> updated{0x99, 0x98};

    REQUIRE(cache.put(keyFor("f22.pt"), kBytes));
    REQUIRE(cache.put(keyFor("f22.pt"), updated)); // temp-file + rename path, twice on one key
    CHECK(*cache.get(keyFor("f22.pt")) == updated);
}

TEST_CASE("TranslationCache sanitizes FA prefix characters in file names") {
    TempDir tmp("cache");
    fa::TranslationCache cache(tmp.path());

    REQUIRE(cache.put(keyFor("&aftb2.11k"), kBytes));
    CHECK(*cache.get(keyFor("&aftb2.11k")) == kBytes);

    // No on-disk name may carry the raw '&' (ealib_safe_name maps it to '_').
    for (const auto& entry : std::filesystem::recursive_directory_iterator(tmp.path()))
        CHECK(entry.path().filename().string().find('&') == std::string::npos);
}

TEST_CASE("TranslationCache degrades to disabled no-ops") {
    SECTION("default constructed") {
        fa::TranslationCache cache;
        CHECK_FALSE(cache.enabled());
        CHECK_FALSE(cache.put(keyFor("f22.pt"), kBytes));
        CHECK_FALSE(cache.get(keyFor("f22.pt")).has_value());
    }
    SECTION("empty root") {
        fa::TranslationCache cache{std::filesystem::path()};
        CHECK_FALSE(cache.enabled());
    }
#if !defined(_WIN32)
    SECTION("unwritable root") {
        if (::geteuid() == 0)
            SKIP("running as root: permission bits do not apply");
        TempDir tmp("cache");
        const auto locked = tmp.path() / "locked";
        std::filesystem::create_directories(locked);
        ::chmod(locked.c_str(), 0500);
        fa::TranslationCache cache(locked / "cache-root");
        CHECK_FALSE(cache.enabled());
        CHECK_FALSE(cache.put(keyFor("f22.pt"), kBytes));
        ::chmod(locked.c_str(), 0700);
    }
#endif
}

TEST_CASE("readWithCache serves raw entries off the map and caches nothing") {
    TempDir install("cache-install");
    TempDir cacheRoot("cache-root");
    fatest::writeLib(install.path() / "FA_1.LIB", {{"F22.PT", {1, 2, 3}}});

    fa::FaVfs vfs;
    REQUIRE(vfs.mount(install.path()));
    fa::TranslationCache cache(cacheRoot.path());

    const auto ref = vfs.find("f22.pt");
    REQUIRE(ref.has_value());
    CHECK(fa::readWithCache(vfs, cache, *ref) == std::vector<uint8_t>{1, 2, 3});
    // flags-0 entries are zero-copy off the map — nothing may be cached.
    CHECK_FALSE(std::filesystem::exists(cacheRoot.path() / "extract"));
}

TEST_CASE("readWithCache hits a seeded cache for a compressed entry") {
    TempDir install("cache-install");
    TempDir cacheRoot("cache-root");
    const auto libFile = install.path() / "FA_1.LIB";
    fatest::writeLib(libFile, {{"SKIN.PIC", {7, 7, 7}}});
    // Mark the entry DCL-compressed. Its payload is NOT valid DCL, so a cache
    // hit is the only way readWithCache can return bytes — which is exactly
    // what this test proves.
    fatest::patchEntryFlags(libFile, 0, 4);

    fa::FaVfs vfs;
    REQUIRE(vfs.mount(install.path()));
    fa::TranslationCache cache(cacheRoot.path());

    const auto ref = vfs.find("skin.pic");
    REQUIRE(ref.has_value());
    REQUIRE(ref->entry->flags == 4);

    const fa::TranslationCache::Key key{"extract", "skin.pic", ref->lib->fileSize(), ref->lib->mtime()};
    REQUIRE(cache.put(key, kBytes));

    bool unsupported = true;
    CHECK(fa::readWithCache(vfs, cache, *ref, &unsupported) == kBytes);
    CHECK_FALSE(unsupported);
}

TEST_CASE("readWithCache never caches a failed extraction") {
    TempDir install("cache-install");
    TempDir cacheRoot("cache-root");
    const auto libFile = install.path() / "FA_1.LIB";
    fatest::writeLib(libFile, {{"SKIN.PIC", {7, 7, 7}}});
    fatest::patchEntryFlags(libFile, 0, 4); // bogus DCL payload -> extraction fails

    fa::FaVfs vfs;
    REQUIRE(vfs.mount(install.path()));
    fa::TranslationCache cache(cacheRoot.path());

    const auto ref = vfs.find("skin.pic");
    REQUIRE(ref.has_value());
    CHECK(fa::readWithCache(vfs, cache, *ref).empty());
    CHECK_FALSE(std::filesystem::exists(cacheRoot.path() / "extract"));
}
