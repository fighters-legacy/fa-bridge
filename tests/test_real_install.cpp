// SPDX-License-Identifier: GPL-3.0-or-later

// Integration against a licensed FA install; skipped unless FA_INSTALL_DIR is
// set (CI never sets it — synthetic fixtures cover the logic; this proves the
// real data path: mixed-case lib names, DCL-compressed entries, scale).

#include "FaAssetTypes.h"
#include "FaContentPack.h"
#include "FaVfs.h"
#include "TestEnv.h"

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
    bool extractedOne = false;
    for (size_t li = 0; li < vfs.libCount() && !extractedOne; ++li) {
        // Walk entries via the public surface: list every mapped extension and
        // read the first compressed hit.
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
        break; // listStems already spans all libs; one pass suffices
    }
    CHECK(extractedOne);
}
