// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "FaVfs.h"
#include "content/IContentPack.h"

#include <filesystem>

namespace fa {

// The FA bridge pack (roadmap Phase 2): discovers and mounts the user's FA
// install and answers hasAsset/listAssets from the mounted archives. The
// load*() transcode pipelines land in roadmap Phase 3 — until then every
// loader returns nullopt and the engine falls through to lower-priority packs.
class FaContentPack : public fl::IContentPack {
  public:
    // Above fl-base-pack (50) so bridged FA content overrides the engine's
    // free base pack in the priority stack.
    static constexpr int kPriority = 100;

    const char* name() const override;
    const char* version() const override;
    const char* id() const override;
    const char* namespaceId() const override;
    int priority() const override;
    const char* rootDirectory() const override;

    Status init() override;
    bool configure(fl::IWindow* window) override;

    bool hasAsset(const char* name, fl::AssetType type) const override;

    std::optional<fl::MeshData> loadMesh(const char* name) override;
    std::optional<fl::TextureData> loadTexture(const char* name) override;
    std::optional<fl::AudioBuffer> loadAudio(const char* name) override;
    std::optional<fl::FlightModel> loadFlightModel(const char* name) override;
    std::optional<fl::MissionData> loadMission(const char* name) override;
    std::optional<fl::TerrainData> loadTerrain(const char* name) override;
    std::optional<fl::AIScript> loadAIScript(const char* name) override;
    std::optional<fl::EntityDefData> loadEntityDef(const char* name) override;
    std::optional<fl::SensorDefData> loadSensorDef(const char* name) override;
    std::optional<fl::WeaponDefData> loadWeaponDef(const char* name) override;
    std::optional<fl::ManualProse> loadManualProse(const char* name) override;

    std::vector<std::string> listAssets(fl::AssetType type) const override;

    std::optional<std::string> loadConfig(const char* name) const override;

    std::optional<std::string> resolveTilePath(const char* terrainId, uint8_t face, uint8_t level, uint32_t i,
                                               uint32_t j, fl::TileLayer layer) const override;

    fl::TrustLevel getTrustLevel() const override;
    bool isNativePlugin() const override;

  private:
    std::filesystem::path m_installDir;
    FaVfs m_vfs;
};

} // namespace fa
