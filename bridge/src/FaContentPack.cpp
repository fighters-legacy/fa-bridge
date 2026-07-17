// SPDX-License-Identifier: GPL-3.0-or-later
#include "FaContentPack.h"

#include "BridgeConfig.h"
#include "FaBridgeVersion.h"
#include "FaInstallLocator.h"
#include "platform/IWindow.h"

#include <string>
#include <utility>

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
    // Discovery chain: FA_INSTALL_DIR env -> persisted config -> Windows
    // registry/drive probes. A candidate must contain at least one .LIB
    // (matched case-insensitively) to count as an FA install.
    if (auto dir = FaInstallLocator::discover(BridgeConfig::defaultPath())) {
        m_installDir = std::move(*dir);
        return Status::Ready;
    }
    m_installDir.clear();
    return Status::NeedsConfiguration;
}

bool FaContentPack::configure(fl::IWindow* window) {
    if (window == nullptr)
        return false;

    std::string lastAttempt;
    for (;;) {
        const auto picked = window->showFolderDialog("Locate your Fighters Anthology installation",
                                                     lastAttempt.empty() ? nullptr : lastAttempt.c_str());
        if (!picked)
            return false; // cancelled, dialog error, or a backend with no picker

        std::filesystem::path dir(*picked);
        if (FaInstallLocator::isValidInstallDir(dir)) {
            BridgeConfig config{dir};
            const auto file = BridgeConfig::defaultPath();
            if (file.empty() || !config.save(file)) {
                // Non-fatal: the pack works this session; the next launch
                // re-runs discovery and may ask again.
                const fl::IWindow::MessageBoxButton ok[] = {{0, "OK"}};
                window->showMessageBox(fl::IWindow::MessageBoxType::Warning, "Fighters Anthology Bridge",
                                       "The install location could not be saved and may be asked for "
                                       "again next launch.",
                                       ok, 1);
            }
            m_installDir = std::move(dir);
            return true;
        }

        lastAttempt = *picked;
        const std::string message = *picked + "\n\nNo .LIB archives were found in this folder. Select the folder "
                                              "that contains the Fighters Anthology archives (e.g. FA_1.LIB).";
        const fl::IWindow::MessageBoxButton buttons[] = {{0, "Retry"}, {1, "Cancel"}};
        const int clicked = window->showMessageBox(
            fl::IWindow::MessageBoxType::Warning, "Not a Fighters Anthology installation", message.c_str(), buttons, 2);
        if (clicked != 0)
            return false; // Cancel button, or -1 error/dismiss
    }
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
