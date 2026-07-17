// SPDX-License-Identifier: GPL-3.0-or-later

// Pack-level hasAsset/listAssets over a mounted synthetic install, plus the
// table-lock test that keeps FaAssetTypes honest against the support matrix.

#include "FaAssetTypes.h"
#include "FaContentPack.h"
#include "SyntheticLib.h"
#include "TempDir.h"
#include "TestEnv.h"

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

using fatest::HermeticEnv;
using fatest::TempDir;

namespace {

// One synthetic install covering every mapped type plus unmapped extensions.
void writeInstall(const std::filesystem::path& dir) {
    fatest::writeLib(dir / "FA_1.LIB", {
                                           {"F22.SH", {1}},      // Mesh
                                           {"SKIN.PIC", {2}},    // Texture
                                           {"SHOT.RAW", {3}},    // Texture
                                           {"GUN.11K", {4}},     // Audio
                                           {"F22.PT", {5}},      // FlightModel
                                           {"BALTIC.M", {6}},    // Mission
                                           {"BALTIC.MM", {7}},   // Mission (map)
                                           {"TANK.OT", {8}},     // EntityDef
                                           {"SHIP.NT", {9}},     // EntityDef
                                           {"DROP.GAS", {10}},   // EntityDef
                                           {"APG68.SEE", {11}},  // SensorDef
                                           {"JAMMER.ECM", {12}}, // SensorDef
                                           {"AIM9.JT", {13}},    // Weapon
                                           {"MENU.DLG", {14}},   // unmapped extension
                                       });
}

fl::AssetType assetType(uint8_t v) {
    return static_cast<fl::AssetType>(v);
}

} // namespace

TEST_CASE("pack hasAsset and listAssets answer from the mounted install") {
    HermeticEnv env;
    TempDir install("pack");
    writeInstall(install.path());
    fatest::setEnv("FA_INSTALL_DIR", install.path().string().c_str());

    fa::FaContentPack pack;
    REQUIRE(pack.init() == fl::IContentPack::Status::Ready);

    SECTION("hasAsset finds stems case insensitively") {
        CHECK(pack.hasAsset("f22", fl::AssetType::FlightModel));
        CHECK(pack.hasAsset("F22", fl::AssetType::FlightModel)); // raw name, engine does not fold
        CHECK(pack.hasAsset("f22", fl::AssetType::Mesh));
        CHECK(pack.hasAsset("aim9", fl::AssetType::Weapon));
        CHECK(pack.hasAsset("apg68", fl::AssetType::SensorDef));
        CHECK(pack.hasAsset("baltic", fl::AssetType::Mission));
    }

    SECTION("hasAsset is false for unknown names and unmapped types") {
        CHECK_FALSE(pack.hasAsset("f35", fl::AssetType::FlightModel));
        CHECK_FALSE(pack.hasAsset("f22", fl::AssetType::Terrain));
        CHECK_FALSE(pack.hasAsset("f22", fl::AssetType::AIScript));
        CHECK_FALSE(pack.hasAsset("menu", fl::AssetType::Texture)); // .DLG is not a texture
        CHECK_FALSE(pack.hasAsset(nullptr, fl::AssetType::Mesh));
    }

    SECTION("listAssets returns lowercase stems per mapped type") {
        CHECK(pack.listAssets(fl::AssetType::Mesh) == std::vector<std::string>{"f22"});
        CHECK(pack.listAssets(fl::AssetType::Texture) == std::vector<std::string>{"shot", "skin"});
        CHECK(pack.listAssets(fl::AssetType::Audio) == std::vector<std::string>{"gun"});
        CHECK(pack.listAssets(fl::AssetType::FlightModel) == std::vector<std::string>{"f22"});
        CHECK(pack.listAssets(fl::AssetType::Mission) == std::vector<std::string>{"baltic"});
        CHECK(pack.listAssets(fl::AssetType::EntityDef) == (std::vector<std::string>{"drop", "ship", "tank"}));
        CHECK(pack.listAssets(fl::AssetType::SensorDef) == (std::vector<std::string>{"apg68", "jammer"}));
        CHECK(pack.listAssets(fl::AssetType::Weapon) == std::vector<std::string>{"aim9"});
        CHECK(pack.listAssets(fl::AssetType::Terrain).empty());
        CHECK(pack.listAssets(fl::AssetType::AIScript).empty());
        CHECK(pack.listAssets(fl::AssetType::Manual).empty());
        CHECK(pack.listAssets(fl::AssetType::Livery).empty());
    }

    SECTION("load methods still return nullopt for present assets") {
        // Phase 2 contract: the engine falls through to lower-priority packs.
        CHECK_FALSE(pack.loadFlightModel("f22"));
        CHECK_FALSE(pack.loadMesh("f22"));
        CHECK_FALSE(pack.loadWeaponDef("aim9"));
    }
}

TEST_CASE("FaAssetTypes maps exactly the documented extensions") {
    // Table-lock against docs/asset-support-matrix.md — update both together.
    const auto expect = [](fl::AssetType type, const std::vector<std::string>& want) {
        const auto span = fa::extensionsFor(type);
        std::vector<std::string> got(span.begin(), span.end());
        CHECK(got == want);
    };
    expect(fl::AssetType::Mesh, {"sh"});
    expect(fl::AssetType::Texture, {"pic", "raw"});
    expect(fl::AssetType::Audio, {"11k", "8k", "5k", "22k"});
    expect(fl::AssetType::FlightModel, {"pt"});
    expect(fl::AssetType::Mission, {"m", "mm"});
    expect(fl::AssetType::EntityDef, {"ot", "nt", "gas"});
    expect(fl::AssetType::SensorDef, {"see", "ecm"});
    expect(fl::AssetType::Weapon, {"jt"});
    expect(fl::AssetType::Terrain, {});
    expect(fl::AssetType::AIScript, {});
    expect(fl::AssetType::Manual, {});
    expect(fl::AssetType::Livery, {});

    // Every enum value below Count is covered above; a new engine asset type
    // must get an explicit row here and in the matrix.
    for (uint8_t t = 0; t < static_cast<uint8_t>(fl::AssetType::Count); ++t)
        (void)fa::extensionsFor(assetType(t)); // must not crash for any value
}
