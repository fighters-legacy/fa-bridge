# FA Asset Support Matrix

The living answer to "which FA content can the bridge deliver to the engine,
and what stands in the way of the rest." Cross-references what
[fx_lib](https://github.com/jomkz/fighters-codex) can decode (pinned at
`extern/fx_lib`, currently v0.8.7) against what the
[fighters-legacy](https://github.com/fighters-legacy/fighters-legacy) engine
can consume, and names the transcoding work this repository uniquely owns.

## Purpose & legend

| Verdict | Meaning |
|---|---|
| **Ready** | Format fully understood upstream and a working engine pipeline exists. fx_lib compiles and links here on every platform — bridge work can start now. |
| **Partial** | A usable subset works today; the gap is documented with its blocker. |
| **Blocked (repo)** | Not usable yet; the named repository owns the blocker. |
| **No engine target** | fx_lib decodes it, but the engine has no consuming subsystem yet. |

**Global gate — lifted.** fx_lib builds cross-platform upstream (fighters-codex
Phase 1) and this repo links it unconditionally on all three CI platforms; the
remaining upstream macOS gap
([codex#155](https://github.com/jomkz/fighters-codex/issues/155)) is proven
here, with fixes flowing upstream. *Ready* verdicts no longer inherit any
build gate — bridge transcoder work can start on every *Ready* row.

## Start-now summary

Content we already have enough information to start bringing into the game:

1. **Aircraft flight models** — every aircraft's thrust, max speed, ceiling,
   fuel, stall, G-envelope, hardpoints, and system damage (`.PT` records) →
   engine FlightModel TOML. The engine flies these end-to-end today.
2. **Entity definitions** — weapons, sensors, ECM, fuel tanks (`.OT`-family
   records) → engine EntityDef TOML.
3. **3D models** — aircraft, ground vehicles, ships, buildings, weapons
   (`.SH`) → glTF `.glb`. 94.9% of the 1,275 stock files decode cleanly.
4. **All 2D art** — cockpit art, menu screens, skins, UI images
   (`.PIC`+`.PAL`, `.RAW`) → PNG, full RGBA8 including transparency.
5. **Sound effects and speech** — PCM audio (`.11K`/`.8K`/`.5K`/`.22K`) →
   OGG Vorbis.
6. **Archive access** — `.LIB` extraction, the prerequisite for all of the
   above.

The definitive per-aircraft roster (which named aircraft have both a
decodable `.SH` and a `.PT` record) requires running `fx` against a licensed
FA install — an early Phase 3 deliverable.

## Matrix

| FA format | fx_lib API | fx_lib status (v0.8.7) | Engine asset type | Canonical format | Bridge transcode | Verdict | Blocked by |
|---|---|---|---|---|---|---|---|
| `.LIB` archives | `fx/ealib.h` | Ready — extract/build/patch, round-trip tested | (container) | — | **Done** — mounted read-only in-plugin VFS: case-insensitive, memory-mapped, all install libs, `hasAsset`/`listAssets` answer from it | **Ready** | — |
| `.PIC` + `.PAL` images | `fx/pic.h`, `fx/pal.h` | Ready — all 3 sub-formats → RGBA8, tested | Texture | PNG (KTX2 later) | RGBA8 → PNG encode | **Ready** | — |
| `.RAW` screenshots | `fx/raw.h` | Ready — RGBA8 via embedded palette | Texture | PNG | RGBA8 → PNG encode | **Ready** | — |
| `.SH` 3D models | `fx/sh.h` | 94.9% of 1,275 files → vertices/faces/textures, now state-aware (`ShState` LOD/damage/frame selection, `sh_articulations()` moving-part selectors); 65 files remain x86 machine-code geometry (incl. some flyables, e.g. `A10.SH`) | Mesh | glTF `.glb` | ShMesh → glb writer; feet → metres | **Ready** (94.9%) | Remainder: fighters-codex Phase 5 ([#52](https://github.com/jomkz/fighters-codex/issues/52)/[#53](https://github.com/jomkz/fighters-codex/issues/53)) |
| `.PT` flight models | `fx/ot.h`, `fx/brf.h` | Ready — named fields, round-trip proven | FlightModel | TOML | Field mapping + TOML emit | **Ready** | — |
| `.OT`/`.NT`/`.GAS` type records | `fx/ot.h`, `fx/brf.h` | Ready — named fields, round-trip proven | EntityDef | TOML | Field mapping + TOML emit | **Ready** | — |
| `.JT` weapon (ordnance) records | `fx/ot.h`, `fx/brf.h` | Ready — named fields, round-trip proven | Weapon | TOML | Field mapping + TOML emit | **Ready** | — |
| `.SEE`/`.ECM` sensor & countermeasure records | `fx/ot.h`, `fx/brf.h` | Ready — named fields, round-trip proven | SensorDef | TOML | Field mapping + TOML emit | **Ready** | — |
| `.11K`/`.8K`/`.5K`/`.22K` PCM audio | `fx/audio.h` | Ready — WAV/PCM, round-trip tested | Audio | OGG Vorbis | PCM → Vorbis encode | **Ready** | — |
| `.M`/`.MM` missions & maps | `fx/mission.h` | Partial — summary info + byte-identical round-trip of all 592 stock missions; **no per-object list** | Mission | YAML | Listing/metadata now; full conversion blocked | **Partial** | Object-list extraction ([codex#156](https://github.com/jomkz/fighters-codex/issues/156)); engine mission runtime ([engine#632](https://github.com/fighters-legacy/fighters-legacy/issues/632)) |
| `.T2` terrain | `fx/t2.h` | Full leaf + tile-summary read/write, byte-identical repack — but `elevation` is a band byte, not a continuous heightmap | Terrain (cube-sphere tiles via `resolveTilePath`) | 16-bit heightmap PNG tiles | Blocked: band→elevation semantics + cube-sphere tiling | **Blocked (fighters-codex)** | Elevation-band semantics ([codex#158](https://github.com/jomkz/fighters-codex/issues/158)) |
| `.SEQ` cutscene timelines | `fx/seq.h` | Ready — round-trip tested | — | — | — | **No engine target** | Engine cutscene subsystem |
| `.CB8` FMV video | `fx/cb8.h` | Ready — frame decode (CLI-proven) | — | — | — | **No engine target** | Engine video subsystem |
| `.PLT`/`.P` pilot saves | `fx/plt.h` | Ready — identity/campaign/stats, tested | — | — | — | **No engine target** | No engine consumer |
| `.FNT`/`.HUD`/`.INF`/`.LAY` overlays | `fx/fnt.h` etc. | Partial — parse/dump, untested | — (engine UI is native) | — | — | **Blocked (fighters-codex)** / no engine target | Upstream test coverage |
| `.XMI`/`.MUS` music | `fx/xmi.h`, `fx/mus.h` | In-lib since v0.8.x: XMI→SMF converter + MUS playlist disassembler (codex#157/#106 delivered) | Audio | OGG Vorbis | XMI→SMF (fx) → MIDI render → OGG; renderer (e.g. FluidSynth) is a new bridge dependency decision | **Ready** (render dep pending) | — |
| `.VDO`/`.FBC` briefing video | `fx/vdo.h`, `fx/fbc.h` | Container parsers in-lib since v0.8.x; full decode is the codex "long pole" | — | — | — | **No engine target** | Engine video subsystem; codec completion ([#55](https://github.com/jomkz/fighters-codex/issues/55)) |
| `.CAM`/`.MC`/`.MT` campaign & briefings | `fx/cam.h`, `fx/mc.h`, `fx/mt.h` | Parsers in-lib since v0.8.x | `loadConfig` at most | — | — | **No engine target** | Engine campaign subsystem |
| `.DLG`/`.MNU`/`.PTS`/`.HGR` UI layouts | `fx/dlg.h`, `fx/mnu.h`, `fx/pts.h`, `fx/hgr.h` | Parsers in-lib since v0.8.x | — (engine UI is native) | — | — | **No engine target** | — |
| AI scripts | `fx/ai.h`, `fx/bi.h` | Compiler + disassembler exist | AIScript | Lua **source** (bytecode rejected) | None — FA AI is bytecode; engine AI is authored fresh in Lua, informed by FA data | **Authored content, not a transcode** | — |
| `fx/sms.h`, `fx/pe.h` (RE tooling) | — | Present in fx_lib | — | — | — | **Assess** — confirm against the codex status matrix | — |

## Category notes

- **Engine validation:** the engine's `AssetValidator` checks every returned
  asset: meshes must carry the `glTF` magic (or lead with `{`), textures
  PNG/KTX2 magic, audio the `OggS` magic, AI scripts must be Lua *source*
  (the `0x1b` bytecode magic is rejected), flight models and entity defs
  TOML text, missions YAML text. The bridge must transcode **before**
  returning bytes — raw FA data is discarded and logged.
- **The 65 machine-code `.SH` files:** ~5% of stock geometry is implemented
  as x86 routines rather than data, including some player-flyable aircraft.
  Decoding them is fighters-codex Phase 5 work; until then those aircraft
  bridge with stats but placeholder geometry.
- **`.LIB` compression coverage:** fx_lib auto-decompresses DCL-compressed
  and raw entries; rare LZSS/PXPK entries are returned still compressed
  ([codex#159](https://github.com/jomkz/fighters-codex/issues/159)).
- **Audio sample rates:** extension encodes the rate; `.22K` → 22050 Hz was
  fixed upstream in fighters-codex #100 (included in v0.3.0).
- **Case sensitivity — implemented:** FA installs vary in filename case; the
  Phase 2 discovery/mount layer matches all FA file names case-insensitively
  by enumeration (ASCII fold; never a literal-path lookup).
- **Type mapping:** the engine's v0.3.x asset-type split re-homes the
  OT-family: `.JT` → Weapon, `.SEE`/`.ECM` → SensorDef, `.OT`/`.NT`/`.GAS` →
  EntityDef. The in-code table (`bridge/src/FaAssetTypes.cpp`) mirrors these
  rows and a table-lock test fails if the two drift. Terrain, AIScript,
  Manual, and Livery deliberately map to no FA extension (no heightmap read
  path yet; engine AI is authored Lua; Manual/Livery are authored content).
- **Cross-LIB precedence:** the mount builds one flat name index like FA's own
  `LibStartUp` hint index (fighters-codex `docs/fa/memory-resource.md`) —
  duplicate names resolve to the last registration. FA registers in OS
  enumeration order; the bridge mounts in case-folded filename order so the
  result is deterministic. FA's loose-file fallback layer is not mounted.
- **Disk-resident libs:** several stock archives ship on the CDs, not the
  install dir (`FA_3`, `FA_7`, `FA_10*`, `FA_11*` — see LIB.md); the mount
  serves whatever is present, so content coverage tracks how full the user's
  install is.

## Maintenance

Docs-currency rule: this matrix is updated **in the same PR** as any change
that shifts a row — a new transcoder, an `extern/fx_lib` bump, or an engine
pipeline landing.
