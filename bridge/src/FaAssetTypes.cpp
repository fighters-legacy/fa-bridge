// SPDX-License-Identifier: GPL-3.0-or-later
#include "FaAssetTypes.h"

namespace fa {

namespace {

using namespace std::string_view_literals;

constexpr std::string_view kMesh[] = {"sh"sv};
constexpr std::string_view kTexture[] = {"pic"sv, "raw"sv};               // .PAL is a .PIC companion, not an asset
constexpr std::string_view kAudio[] = {"11k"sv, "8k"sv, "5k"sv, "22k"sv}; // .XMI/.MUS wait on codex#157/#106
constexpr std::string_view kFlightModel[] = {"pt"sv};
constexpr std::string_view kMission[] = {"m"sv, "mm"sv};
constexpr std::string_view kEntityDef[] = {"ot"sv, "nt"sv, "gas"sv};
constexpr std::string_view kSensorDef[] = {"see"sv, "ecm"sv};
constexpr std::string_view kWeapon[] = {"jt"sv};

} // namespace

std::span<const std::string_view> extensionsFor(fl::AssetType type) {
    switch (type) {
    case fl::AssetType::Mesh:
        return kMesh;
    case fl::AssetType::Texture:
        return kTexture;
    case fl::AssetType::Audio:
        return kAudio;
    case fl::AssetType::FlightModel:
        return kFlightModel;
    case fl::AssetType::Mission:
        return kMission;
    case fl::AssetType::EntityDef:
        return kEntityDef;
    case fl::AssetType::SensorDef:
        return kSensorDef;
    case fl::AssetType::Weapon:
        return kWeapon;
    case fl::AssetType::Terrain:
    case fl::AssetType::AIScript:
    case fl::AssetType::Manual:
    case fl::AssetType::Livery:
    case fl::AssetType::Count:
        break;
    }
    return {};
}

} // namespace fa
