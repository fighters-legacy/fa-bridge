# Roadmap

Development is tracked through [GitHub milestones](https://github.com/fighters-legacy/fa-bridge/milestones),
one per phase. Each phase is an `epic`-labeled issue whose native sub-issues carry the work.
This document is the map; the tracker is the source of truth for status.

## Charter

**The working plugin is the primary output.** fa-bridge implements the fighters-legacy
engine's `IContentPack` and transcodes Jane's Fighters Anthology (1998) assets into the
engine's canonical open formats at load time. Reverse-engineering knowledge is *consumed*
from [fighters-codex](https://github.com/jomkz/fighters-codex) — via its docs and the
`fx_lib` submodule — and never produced here: when bridging exposes a format gap, it
becomes a fighters-codex issue, not local RE.

## The 1.0 definition

v1.0.0 ships when, and only when:

1. **Every category in [asset-support-matrix.md](asset-support-matrix.md) is bridged**,
   or carries a written rationale (no engine target / upstream-blocked with issue link /
   out of scope).
2. **A stock FA install loads end-to-end** on Linux and Windows: flyable aircraft with
   textures and sound; missions playable via the engine's mission runtime.
3. **Every transcoder has tests and synthetic fixtures** (no FA assets committed) and its
   output passes engine `AssetValidator` conformance.
4. **License and distribution positioning is resolved** and documented
   ([#37](https://github.com/fighters-legacy/fa-bridge/issues/37)).
5. Pins (`extern/fx_lib`, engine interface headers) are current and documented; releases
   current; the project then enters maintenance mode.

## Phases

Phases are **gated by ordering constraints, not dates**.

```
P0 ─→ P1 ─→ P2 ─→ P3 ─→ P4 ──────────────→ P6 (1.0)
             ▲          ▲                   ▲
 codex P1 ───┘          │                   │
 (fx_lib on Linux)      │                   │
 engine glTF materials ─┘ (textured accept) │
                        P5 ─────────────────┘
                         ▲
   codex mission object-list + engine mission runtime + codex T2 heightmaps

P1 may start any time after P0 — it needs no fx_lib (vendored engine headers only).
The license decision (#37) is owner-external — must resolve before P4's public release.
```

| Phase | Milestone | Epic | Goal | Exit gate | External gates |
|---|---|---|---|---|---|
| **0 — Program Reset** | [Phase 0](https://github.com/fighters-legacy/fa-bridge/milestone/2) | [#30](https://github.com/fighters-legacy/fa-bridge/issues/30) | Roadmap + asset matrix live, claims true, submodule pinned, tooling portable, v0.1.0 | README contradicts nothing | — |
| **1 — Plugin Skeleton** | [Phase 1](https://github.com/fighters-legacy/fa-bridge/milestone/3) | [#31](https://github.com/fighters-legacy/fa-bridge/issues/31) | Compiling `libfa-bridge.so`/`.dll`/`.dylib`: stub `IContentPack`, `fighters_legacy_create_pack()`, `manifest.toml`, smoke tests, real CI | Engine `ModLoader` loads the stub on Fedora | Engine-header pin only — **unblocked now** |
| **2 — fx_lib Integration** | [Phase 2](https://github.com/fighters-legacy/fa-bridge/milestone/4) | [#32](https://github.com/fighters-legacy/fa-bridge/issues/32) | fx_lib linked unconditionally (former `FA_WITH_FX_LIB` option retired); FA-install discovery (case-insensitive lookup, `init()`/`configure()` flow); LIB mounting; translation cache | fx_lib-linked build green on all 3 OSes | fighters-codex Phase 1 ([codex#42](https://github.com/jomkz/fighters-codex/issues/42)) — **landed**; macOS gap ([codex#155](https://github.com/jomkz/fighters-codex/issues/155)) proven by this repo's CI |
| **3 — Ready-Now Bridging** | [Phase 3](https://github.com/fighters-legacy/fa-bridge/milestone/5) | [#33](https://github.com/fighters-legacy/fa-bridge/issues/33) | The six Ready pipelines: PIC/PAL/RAW→PNG, PCM→OGG, SH→glb, PT→FlightModel TOML, OT-family→EntityDef TOML, LIB mount; per-aircraft roster ([#39](https://github.com/fighters-legacy/fa-bridge/issues/39)) | Each transcoder validator-conformant with fixture tests | fighters-codex Phase 1 |
| **4 — In-Engine Validation** | [Phase 4](https://github.com/fighters-legacy/fa-bridge/milestone/6) | [#34](https://github.com/fighters-legacy/fa-bridge/issues/34) | FA aircraft visible + audible in fighters-legacy; priority vs fl-base-pack; `configure()` first-run UI; first usable release | Owner flies an FA aircraft on Fedora | Engine glTF materials for *textured* acceptance ([engine#658](https://github.com/fighters-legacy/fighters-legacy/issues/658)); license decision ([#37](https://github.com/fighters-legacy/fa-bridge/issues/37)) |
| **5 — Missions & Terrain** | [Phase 5](https://github.com/fighters-legacy/fa-bridge/milestone/7) | [#35](https://github.com/fighters-legacy/fa-bridge/issues/35) | FA mission→YAML; terrain chunks via `resolveTerrainChunk` | Stock FA mission playable | Mission object-list ([codex#156](https://github.com/jomkz/fighters-codex/issues/156)); engine mission runtime ([engine#632](https://github.com/fighters-legacy/fighters-legacy/issues/632)); T2 heightmaps ([codex#158](https://github.com/jomkz/fighters-codex/issues/158)) |
| **6 — Full Coverage & 1.0** | [Phase 6](https://github.com/fighters-legacy/fa-bridge/milestone/8) | [#36](https://github.com/fighters-legacy/fa-bridge/issues/36) | Music, video/cutscenes, campaign, the 65 machine-code SHs, UI formats — bridged or rationale'd | The 1.0 definition; v1.0.0 tagged | fighters-codex Phase 4/5 ([codex#52](https://github.com/jomkz/fighters-codex/issues/52), [codex#53](https://github.com/jomkz/fighters-codex/issues/53), [codex#55](https://github.com/jomkz/fighters-codex/issues/55) — music lib-ification landed in v0.8.x) |

## What is unblocked today

- **Phases 1 and 2 are delivered** — the plugin skeleton (v0.2.x) and the fx_lib
  integration: unconditional linkage, install discovery + first-run `configure()`,
  the LIB mount answering `hasAsset`/`listAssets`, and the translation cache.
- **All of Phase 3** — every *Ready* matrix row can grow its transcoder now: fx_lib
  links on every platform, the mount and cache are in place, and the write-side
  library decisions (below) are the remaining choices.

## Platform notes

- **macOS gap upstream:** fighters-codex Phase 1's exit gate covers Linux (GCC/Clang) and
  Windows (MSVC) only; this repo's rule is all three platforms including AppleClang.
  Tracked upstream as [codex#155](https://github.com/jomkz/fighters-codex/issues/155),
  with this repo's 3-OS CI as the proving ground now that fx_lib is linked unconditionally.
- **Plugin ABI discipline:** `std::optional`/`std::string`/`std::vector` cross the plugin
  boundary, so the plugin must be built with the same toolchain family and CRT
  configuration as the engine (MSVC: matching `/MD` and iterator-debug-level; Linux:
  libstdc++ vs libc++). The factory symbol needs `__declspec(dllexport)` under MSVC.
  See [architecture.md](architecture.md).

## Bridge-owned transcoding and third-party dependencies

The engine accepts only canonical open formats, so this repository uniquely owns the
write side. Dependency decisions (RFC-visible per [GOVERNANCE.md](../GOVERNANCE.md)
"major new dependencies"):

| Transcode | Dependency | Rationale |
|---|---|---|
| ShMesh → glTF `.glb` | **cgltf + cgltf_write** (MIT, single-header C) | GLB write support; tinygltf rejected as heavier (drags json.hpp + stb) |
| RGBA8 → PNG | **stb_image_write.h** (public domain) | Single header; KTX2/libktx deferred ([#41](https://github.com/fighters-legacy/fa-bridge/issues/41)) |
| PCM → OGG Vorbis | **libogg + libvorbis/vorbisenc** (BSD-3) | The validator requires `OggS` and no credible single-header *encoder* exists; verify minivorbis first ([#40](https://github.com/fighters-legacy/fa-bridge/issues/40)) |
| FlightModel/EntityDef TOML | hand-rolled emitter | Fixed output shape; ~100 lines; no dependency |
| Mission YAML | hand-rolled emitter | Blocked until Phase 5 anyway |

Vendoring layout (Phase 3): single headers under `transcode/vendor/` with REUSE
annotations; libogg/libvorbis under `extern/`.

## Relationship to fighters-codex and fighters-legacy

- **fighters-codex** (MIT) owns all FA reverse engineering; this repo consumes `fx_lib`
  as the `extern/fx_lib` submodule, pinned to a release tag. After each codex release the
  pin is bumped (their release script prints the reminder; dependabot watches the
  submodule). Format gaps found here are filed there.
- **fighters-legacy** (GPL) owns `IContentPack` — the interface source of truth. The
  three interface headers are vendored with a recorded pin (Phase 1) because the engine
  exports no CMake package; the interface may still gain pure virtuals, so the pin is
  load-bearing and bumped deliberately.
- **License boundary and the open question:** fighters-codex is MIT; one-way consumption
  into this GPL repo is fine, and code is never transcribed against that boundary. This
  repository is **GPL-3.0-or-later today** — but the engine's distribution docs have
  positioned fa-bridge as a paid, non-GPL plugin (possible via `IContentPack.h`'s GPL
  linking exception). That decision is deliberately open, tracked in
  [#37](https://github.com/fighters-legacy/fa-bridge/issues/37) (and
  [engine#657](https://github.com/fighters-legacy/fighters-legacy/issues/657)); it gates
  Phase 4's public release. If the proprietary branch is ever chosen, relicensing must
  happen **before** any external DCO-signed contribution is accepted.

## Releases

Minimum one release per phase gate: v0.1.0 (P0) · v0.2.0 (P1) · v0.3.0 (P2) ·
v0.4.0 (P3) · v0.5.0 (P4) · v0.6+ (P5) · **v1.0.0** (P6).

## How this roadmap is maintained

- Phases are milestones; epics are `epic`-labeled issues; work items are native
  sub-issues. New work lands as a sub-issue of the epic it serves.
- [asset-support-matrix.md](asset-support-matrix.md) is updated in the same PR as the
  change it reflects (the docs-currency rule in [CLAUDE.md](../CLAUDE.md)).
- No standalone TODO files — if it's worth doing, it's an issue.
