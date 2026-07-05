# fa-bridge — FA Bridge Plugin

FA Bridge Plugin for [fighters-legacy](https://github.com/fighters-legacy/fighters-legacy).
Implements the engine's `IContentPack` interface, using
[fx_lib](https://github.com/jomkz/fighters-codex) to transcode Jane's Fighters
Anthology (1998) asset formats into the engine's canonical open formats.

## What this is

fighters-legacy is a clean-room GPL v3 reimplementation of the combat flight simulator
engine. It has no knowledge of FA-specific formats — all content enters through
`IContentPack` as raw bytes in canonical formats (glTF, PNG, OGG Vorbis, TOML, YAML).
This repository is one implementation of that interface: a **bridge**, not a content
pack. It ships no assets of its own — it transcodes content from the end-user's own FA
installation into engine-compatible assets, cached locally before first use.

**You must own a legitimate copy of Jane's Fighters Anthology to use this plugin.**
No FA game assets are included or may be committed to this repository.

## Status

**Plugin skeleton (roadmap Phase 1).** The plugin builds on all three platforms as
`libfa-bridge.so` / `fa-bridge.dll` / `libfa-bridge.dylib` with a stub `IContentPack`
that the engine's `ModLoader` can load; it identifies itself and reports readiness from
`FA_INSTALL_DIR`, but serves no assets yet. Transcoding lands per the phase plan in
[docs/roadmap.md](docs/roadmap.md). Which FA content is already fully understood and
ready to bridge — aircraft flight models, entity stats, ~95% of 3D models, all 2D art,
sound effects — is tracked in
[docs/asset-support-matrix.md](docs/asset-support-matrix.md).

## Dependencies

| Dependency | Source |
|---|---|
| fighters-legacy | Engine interface headers (`IContentPack.h`), vendored at `extern/fl-headers`, pinned to engine v0.2.6 ([PIN.md](extern/fl-headers/PIN.md)) |
| fx_lib | `extern/fx_lib` submodule from [fighters-codex](https://github.com/jomkz/fighters-codex), pinned to a release tag |

The reverse-engineering documentation this bridge builds on lives in
[fighters-codex](https://github.com/jomkz/fighters-codex) (the FA reconstruction). Two of its
components are worth noting for the engine-side work: **fx_render** — a shared MIT render module
(OpenGL + FA-faithful software backends), built engine-agnostic so the classic/parity render path can
adopt it instead of a bespoke rasterizer — and **fxc**, a clean-room MIT *source port* of the game
executable. fxc is an **independent sibling** that validates the same reconstruction; it is **not a
dependency** of this bridge.

## Quick start

```bash
git clone --recurse-submodules https://github.com/fighters-legacy/fa-bridge.git
cmake --preset debug
cmake --build --preset debug
```

Note: `fx_lib` builds on Linux (fighters-codex Phase 1 has landed); macOS support is pending
([fighters-codex#155](https://github.com/jomkz/fighters-codex/issues/155)). The `FA_WITH_FX_LIB`
CMake option currently defaults OFF.

See [docs/development.md](docs/development.md) for full prerequisites and build
instructions.

## License

GPL v3. See [LICENSE](LICENSE).

Final distribution positioning (free GPL plugin vs. the paid model sketched in early
engine docs) is an open decision tracked in
[#37](https://github.com/fighters-legacy/fa-bridge/issues/37) and
[docs/roadmap.md](docs/roadmap.md).

> **Never commit FA game assets** (`*.LIB`, `*.PIC`, `*.PAL`, `*.SH`, `*.PT`, `*.OT`,
> `*.M`, `*.MM`, `*.T2`, `*.CB8`, and the rest of the extension set in `.gitignore`).
> They are owned by Electronic Arts and are not covered by this project's license.
