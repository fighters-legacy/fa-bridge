# Architecture

fa-bridge is a native bridge plugin implementing the `IContentPack` interface from
[fighters-legacy](https://github.com/fighters-legacy/fighters-legacy). It transcodes
Jane's Fighters Anthology (1998) asset formats into the engine's canonical open formats
using the FA parsers in [fx_lib](https://github.com/jomkz/fighters-codex).

The engine has no knowledge of FA, this repository, or any specific game. All it knows is
`IContentPack`.

**Status:** `bridge/` and `extern/fl-headers` exist (Phase 1 — a stub pack the engine
loads; it serves no assets yet). `transcode/` and the fx_lib-backed load paths land per
[roadmap.md](roadmap.md).

---

## Data flow

```
FA Install (user-supplied, *.LIB / *.PIC / *.PAL / *.SH / *.PT / ...)
    │
    ▼
extern/fx_lib    (fighters-codex submodule — FA format parsers)
    │  decode raw FA binary formats → fx structs (RGBA8, ShMesh, BRF fields, PCM, ...)
    ▼
transcode/       (canonical-format writers)
    │  RGBA8→PNG · ShMesh→glTF .glb · PCM→OGG Vorbis · PT/OT→TOML · mission→YAML
    ▼
bridge/          (IContentPack implementation)
    │  load*() returns raw bytes in the canonical format
    ▼
Engine AssetValidator → AssetManager   (fighters-legacy)
    │
    ▼
Game runtime
```

The critical contract: the engine's asset structs are `{name, bytes}` and its
`AssetValidator` enforces canonical formats by magic/content — glTF `.glb` for meshes,
PNG/KTX2 for textures, `OggS` for audio, TOML text for flight models and entity defs,
YAML text for missions, Lua *source* for AI (bytecode is rejected). **Every `load*()` is
a transcode, not a pass-through**; raw FA bytes returned to the engine are discarded and
logged.

---

## Plugin mechanics

- The plugin ships as a native shared library named for the manifest `id`:
  `libfa-bridge.so` (Linux), `fa-bridge.dll` (Windows), `libfa-bridge.dylib` (macOS),
  installed beside `manifest.toml` in `mods/fa-bridge/`.
- The engine's `ModLoader` resolves one C symbol:
  `extern "C" fl::IContentPack* fighters_legacy_create_pack();`
  (exported with `__declspec(dllexport)` under MSVC).
- `manifest.toml` declares `name`, `id`, `version`, `engine-api = "1.x"` (major checked),
  and `priority` — set **above 50** so FA content overrides the engine's free
  fl-base-pack in the priority stack.
- Install discovery: `init()` runs a chain — `FA_INSTALL_DIR` env var → the
  persisted config file (`<config dir>/fighters-legacy/fa-bridge/config.toml`;
  platform config dir, `$XDG_CONFIG_HOME`/`%APPDATA%`/`~/Library/Application
  Support`) → Windows-only best-effort probes (registry, then
  `<drive>:\JANES\Fighters Anthology` on fixed drives). A candidate counts as
  an FA install only if it contains at least one `.LIB`, matched
  case-insensitively. First valid candidate wins; none → `NeedsConfiguration`.
- First run: on `NeedsConfiguration` the engine calls `configure(window)`, which
  loops an OS-native folder picker (`IWindow::showFolderDialog`, engine#665)
  with a Retry/Cancel warning box on invalid selections, persists the confirmed
  path to the config file, and returns false on cancel (the engine then drops
  the pack for the session — by contract). Headless hosts pass no window and
  the pack is skipped the same way. Env knobs for tests/dev:
  `FA_BRIDGE_CONFIG_DIR`, `FA_BRIDGE_CACHE_DIR`, `FA_BRIDGE_NO_PROBE`.
- As a native, unsigned plugin the engine surfaces consent prompts
  (`onNativeCodePackLoaded`, `onUntrustedPackLoaded`) — expected behaviour.
- **ABI discipline:** `std::optional`/`std::string`/`std::vector` cross the plugin
  boundary. Build with the same toolchain family and CRT configuration as the engine
  (MSVC: matching `/MD` and iterator-debug-level; Linux: libstdc++ vs libc++).

---

## Key modules

### `bridge/` (exists — stub)

Implements `IContentPack` in `namespace fa` — a static core library
(`fa-bridge-core`, unit-testable without loading the plugin) plus a thin shared-library
shell exporting only `fighters_legacy_create_pack()`. Today every `load*()` returns
nullopt and `init()` reports readiness from `FA_INSTALL_DIR`. From Phase 2/3 on, each
`load*()` method:
1. Locates the relevant file(s) in the mounted FA install (case-insensitive lookup —
   FA installs vary in filename case and Linux filesystems are case-sensitive)
2. Calls the appropriate fx_lib parser to decode the FA binary format
3. Hands the decoded data to `transcode/` and returns canonical-format bytes

A translation cache sits between transcode and the engine — converted assets are cached
on first load so repeated requests (e.g. a texture used by many models) don't re-parse
and re-encode the source each time.

### `transcode/` (planned — Phase 3)

The canonical-format write side this repository uniquely owns: ShMesh→glb (feet→metres),
RGBA8→PNG, PCM→Vorbis, BRF field mapping→TOML. Dependency choices are recorded in
[roadmap.md](roadmap.md). Named `transcode/`, not `assets/` — a directory called
"assets" in a repository whose hardest rule is *never commit assets* is an accident
waiting to happen.

### `extern/fx_lib`

Git submodule from [fighters-codex](https://github.com/jomkz/fighters-codex), pinned to
a release tag. Provides the FA format parsers; the bridge never duplicates parser logic.
What each parser can deliver today is tracked in
[asset-support-matrix.md](asset-support-matrix.md).

### `extern/fl-headers` (exists)

Verbatim copies of the engine's interface headers — the content-pack set
(`IContentPack.h`, `AssetTypes.h`, `TrustLevel.h`) plus the window set the first-run
`configure()` flow calls (`IWindow.h` with its includes `IDisplay.h`,
`IWindowEventHandler.h`) — a closed, std-only set, currently pinned to engine
**v0.3.6**. [PIN.md](../extern/fl-headers/PIN.md) records the tag/SHA, per-file hashes,
the update procedure, and the `IWindow` vtable/ABI note (engine and plugin rebuild
together across window-interface changes). Vendored because the engine exports no
CMake package; the content headers carry a GPL linking exception.

---

## Design constraints

- **One-way bridge.** The engine calls `IContentPack`; it never links against this repo
  or fx_lib directly. The dependency arrow points inward only.
- **No FA assets in the repo.** All FA content is sourced at runtime from the
  user-supplied FA installation. The `.gitignore` blocks the FA extension set (both
  cases); FA files with generic extensions (`.TXT`, `.BIN`, `.DAT`, `.CFG`) are caught in
  review.
- **fx_lib is a submodule, not vendored.** Updates are picked up by bumping the submodule
  pointer to the next codex release — no duplication.
- **`FA_INSTALL_DIR` is required at runtime, not build time.** The bridge builds without
  an FA installation present; tests that need real FA data are skipped when it is unset.
