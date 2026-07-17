// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "content/AssetTypes.h"

#include <span>
#include <string_view>

namespace fa {

// FA source extensions (lowercase, no dot) the bridge serves per engine asset
// type; the order within a list is the lookup precedence. Derived from
// docs/asset-support-matrix.md — the matrix is the authoritative record of
// each mapping and its rationale; a table-lock test keeps the two honest.
// Empty spans are deliberate: Terrain has no heightmap read path yet
// (codex#158 delivers bands, not elevations), engine AI is authored Lua that
// is never transcoded, and Manual/Livery are authored content FA never had.
std::span<const std::string_view> extensionsFor(fl::AssetType type);

} // namespace fa
