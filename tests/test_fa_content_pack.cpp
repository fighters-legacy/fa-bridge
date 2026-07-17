// SPDX-License-Identifier: GPL-3.0-or-later

#include "FaContentPack.h"
#include "TestEnv.h"

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>

using fatest::setEnv;
using fatest::unsetEnv;

TEST_CASE("pack identity is stable") {
    fa::FaContentPack pack;

    CHECK(std::strcmp(pack.id(), "fa-bridge") == 0);
    CHECK(std::strcmp(pack.name(), "Fighters Anthology Bridge") == 0);
    // Def-id namespace: short form per the fl-base precedent, must match the
    // manifest's `namespace` field, never contains ':'.
    CHECK(std::strcmp(pack.namespaceId(), "fa") == 0);
    CHECK(std::strchr(pack.namespaceId(), ':') == nullptr);
    CHECK((pack.version() != nullptr && *pack.version() != '\0'));
    CHECK(pack.priority() == 100);
    CHECK(pack.priority() > 50); // must override fl-base-pack in the stack
    CHECK(pack.rootDirectory() == nullptr);
    CHECK(pack.getTrustLevel() == fl::TrustLevel::Unsigned);
    CHECK(pack.isNativePlugin());
}

TEST_CASE("stub load methods return nullopt") {
    fa::FaContentPack pack;

    CHECK_FALSE(pack.configure(nullptr));
    CHECK_FALSE(pack.loadMesh("any"));
    CHECK_FALSE(pack.loadTexture("any"));
    CHECK_FALSE(pack.loadAudio("any"));
    CHECK_FALSE(pack.loadFlightModel("any"));
    CHECK_FALSE(pack.loadMission("any"));
    CHECK_FALSE(pack.loadTerrain("any"));
    CHECK_FALSE(pack.loadAIScript("any"));
    CHECK_FALSE(pack.loadEntityDef("any"));
    CHECK_FALSE(pack.loadSensorDef("any"));
    CHECK_FALSE(pack.loadWeaponDef("any"));
    CHECK_FALSE(pack.loadManualProse("any"));
    CHECK_FALSE(pack.loadLivery("any"));
    CHECK_FALSE(pack.loadConfig("difficulty.toml"));
    CHECK_FALSE(pack.resolveTilePath("world", 0, 0, 0, 0, fl::TileLayer::Height));
}

TEST_CASE("hasAsset and listAssets are empty for every type") {
    fa::FaContentPack pack;

    for (uint8_t t = 0; t < static_cast<uint8_t>(fl::AssetType::Count); ++t) {
        const auto type = static_cast<fl::AssetType>(t);
        CHECK_FALSE(pack.hasAsset("any", type));
        CHECK(pack.listAssets(type).empty());
    }
}

TEST_CASE("init requires FA_INSTALL_DIR pointing at a directory") {
    fa::FaContentPack pack;

    SECTION("env unset yields NeedsConfiguration") {
        unsetEnv("FA_INSTALL_DIR");
        CHECK(pack.init() == fl::IContentPack::Status::NeedsConfiguration);
    }

    SECTION("non-existent path yields NeedsConfiguration") {
        const auto missing = std::filesystem::temp_directory_path() / "fa-bridge-test-does-not-exist";
        std::filesystem::remove_all(missing);
        setEnv("FA_INSTALL_DIR", missing.string().c_str());
        CHECK(pack.init() == fl::IContentPack::Status::NeedsConfiguration);
    }

    SECTION("existing file yields NeedsConfiguration") {
        const auto file = std::filesystem::temp_directory_path() / "fa-bridge-test-file";
        std::ofstream(file).put('x');
        setEnv("FA_INSTALL_DIR", file.string().c_str());
        CHECK(pack.init() == fl::IContentPack::Status::NeedsConfiguration);
        std::filesystem::remove(file);
    }

    SECTION("existing directory yields Ready, idempotently") {
        const auto dir = std::filesystem::temp_directory_path() / "fa-bridge-test-dir";
        std::filesystem::create_directories(dir);
        setEnv("FA_INSTALL_DIR", dir.string().c_str());
        CHECK(pack.init() == fl::IContentPack::Status::Ready);
        CHECK(pack.init() == fl::IContentPack::Status::Ready);
        std::filesystem::remove_all(dir);
    }

    unsetEnv("FA_INSTALL_DIR");
}
