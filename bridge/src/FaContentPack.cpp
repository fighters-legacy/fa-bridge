// SPDX-License-Identifier: GPL-3.0-or-later
#include "FaContentPack.h"

#include "FaBridgeVersion.h"

#include <cstdlib>

namespace fa {

const char* FaContentPack::name() const {
    return "Fighters Anthology Bridge";
}

const char* FaContentPack::version() const {
    return FA_BRIDGE_VERSION;
}

const char* FaContentPack::id() const {
    // Must stay equal to the manifest id and the shared-library target name:
    // the engine's ModLoader derives the plugin filename from the manifest id.
    return "fa-bridge";
}

const char* FaContentPack::namespaceId() const {
    // Def-id namespace ("fa:<local>"), declared as `[mod] namespace` in the
    // manifest. Deliberately shorter than the manifest id, following the
    // fl-base-pack precedent (manifest id "fl-base-pack", namespace "fl-base").
    return "fa";
}

int FaContentPack::priority() const {
    return kPriority;
}

const char* FaContentPack::rootDirectory() const {
    // No filesystem root: assets are transcoded from the user's FA install,
    // never served from a directory under the engine's Assets domain.
    return nullptr;
}

FaContentPack::Status FaContentPack::init() {
    // Phase 1: ready only when FA_INSTALL_DIR points at an existing directory.
    // The configure() flow that discovers and persists the install is Phase 2.
    const char* dir = std::getenv("FA_INSTALL_DIR");
    if (dir != nullptr && *dir != '\0') {
        std::error_code ec;
        if (std::filesystem::is_directory(dir, ec)) {
            m_installDir = dir;
            return Status::Ready;
        }
    }
    m_installDir.clear();
    return Status::NeedsConfiguration;
}

bool FaContentPack::configure(fl::IWindow*) {
    return false;
}

bool FaContentPack::hasAsset(const char*, fl::AssetType) const {
    return false;
}

std::optional<fl::MeshData> FaContentPack::loadMesh(const char*) {
    return std::nullopt;
}

std::optional<fl::TextureData> FaContentPack::loadTexture(const char*) {
    return std::nullopt;
}

std::optional<fl::AudioBuffer> FaContentPack::loadAudio(const char*) {
    return std::nullopt;
}

std::optional<fl::FlightModel> FaContentPack::loadFlightModel(const char*) {
    return std::nullopt;
}

std::optional<fl::MissionData> FaContentPack::loadMission(const char*) {
    return std::nullopt;
}

std::optional<fl::TerrainData> FaContentPack::loadTerrain(const char*) {
    return std::nullopt;
}

std::optional<fl::AIScript> FaContentPack::loadAIScript(const char*) {
    return std::nullopt;
}

std::optional<fl::EntityDefData> FaContentPack::loadEntityDef(const char*) {
    return std::nullopt;
}

std::optional<fl::SensorDefData> FaContentPack::loadSensorDef(const char*) {
    return std::nullopt;
}

std::optional<fl::WeaponDefData> FaContentPack::loadWeaponDef(const char*) {
    return std::nullopt;
}

std::optional<fl::ManualProse> FaContentPack::loadManualProse(const char*) {
    return std::nullopt;
}

std::vector<std::string> FaContentPack::listAssets(fl::AssetType) const {
    return {};
}

std::optional<std::string> FaContentPack::loadConfig(const char*) const {
    return std::nullopt;
}

std::optional<std::string> FaContentPack::resolveTilePath(const char*, uint8_t, uint8_t, uint32_t, uint32_t,
                                                          fl::TileLayer) const {
    return std::nullopt;
}

fl::TrustLevel FaContentPack::getTrustLevel() const {
    return fl::TrustLevel::Unsigned;
}

bool FaContentPack::isNativePlugin() const {
    return true;
}

} // namespace fa
