// SPDX-License-Identifier: GPL-3.0-or-later

// Integration against a licensed FA install; skipped unless FA_INSTALL_DIR is
// set (CI never sets it — synthetic fixtures cover the logic; this proves the
// real data path: mixed-case lib names, DCL-compressed entries, scale).

#include "FaAssetTypes.h"
#include "FaContentPack.h"
#include "FaVfs.h"
#include "TempDir.h"
#include "TestEnv.h"
#include "TranslationCache.h"

#include <catch2/catch_test_macros.hpp>

#include <cstdlib>
#include <iostream>
#include <string>

namespace {

std::string realInstallOrEmpty() {
    const char* dir = std::getenv("FA_INSTALL_DIR");
    return (dir != nullptr) ? std::string(dir) : std::string();
}

} // namespace

TEST_CASE("real install mounts all libs and lists plausible content", "[integration]") {
    const std::string install = realInstallOrEmpty();
    if (install.empty())
        SKIP("FA_INSTALL_DIR not set");

    fa::FaVfs vfs;
    REQUIRE(vfs.mount(install));
    // Evidence for review: lib/entry counts and duplicate pressure.
    std::cout << "real install: " << vfs.libCount() << " libs, " << vfs.entryCount() << " distinct entries\n";
    CHECK(vfs.libCount() >= 1);
    CHECK(vfs.entryCount() > 100);

    // The pack answers from the same mount.
    fa::FaContentPack pack;
    REQUIRE(pack.init() == fl::IContentPack::Status::Ready);
    const auto flightModels = pack.listAssets(fl::AssetType::FlightModel);
    const auto textures = pack.listAssets(fl::AssetType::Texture);
    const auto audio = pack.listAssets(fl::AssetType::Audio);
    std::cout << "real install: " << flightModels.size() << " flight models, " << textures.size() << " textures, "
              << audio.size() << " sounds\n";
    CHECK_FALSE(flightModels.empty());
    CHECK_FALSE(textures.empty());
    CHECK_FALSE(audio.empty());

    // Every listed stem must satisfy hasAsset (round-trip consistency).
    for (const auto& stem : flightModels)
        REQUIRE(pack.hasAsset(stem.c_str(), fl::AssetType::FlightModel));
}

TEST_CASE("real install extracts a compressed entry", "[integration]") {
    const std::string install = realInstallOrEmpty();
    if (install.empty())
        SKIP("FA_INSTALL_DIR not set");

    fa::FaVfs vfs;
    REQUIRE(vfs.mount(install));

    // Find any DCL-compressed (flags==4) entry and prove blast on real data.
    // listStems already spans every mounted lib, so one pass over the texture
    // stems suffices.
    bool extractedOne = false;
    for (const auto& stem : vfs.listStems(fa::extensionsFor(fl::AssetType::Texture))) {
        const auto ref = vfs.findStem(stem, fa::extensionsFor(fl::AssetType::Texture));
        if (ref && ref->entry->flags == 4) {
            bool unsupported = false;
            const auto bytes = vfs.read(*ref, true, &unsupported);
            CHECK_FALSE(unsupported);
            CHECK_FALSE(bytes.empty());
            extractedOne = true;
            break;
        }
    }
    CHECK(extractedOne);
}

TEST_CASE("real install warm extract hits the cache", "[integration]") {
    const std::string install = realInstallOrEmpty();
    if (install.empty())
        SKIP("FA_INSTALL_DIR not set");

    fa::FaVfs vfs;
    REQUIRE(vfs.mount(install));
    fatest::TempDir cacheRoot("real-cache");
    fa::TranslationCache cache(cacheRoot.path());
    REQUIRE(cache.enabled());

    // First DCL-compressed entry through the cached path, twice.
    for (const auto& stem : vfs.listStems(fa::extensionsFor(fl::AssetType::Texture))) {
        const auto ref = vfs.findStem(stem, fa::extensionsFor(fl::AssetType::Texture));
        if (!ref || ref->entry->flags != 4)
            continue;
        const auto cold = fa::readWithCache(vfs, cache, *ref);
        REQUIRE_FALSE(cold.empty());
        CHECK(std::filesystem::exists(cacheRoot.path() / "extract")); // materialized
        const auto warm = fa::readWithCache(vfs, cache, *ref);
        CHECK(warm == cold); // byte-identical through the hit path
        return;
    }
    FAIL("no compressed texture entry found in the install");
}
