# fa-content — FA Bridge Plugin

FA Bridge Plugin for [fighters-legacy](https://github.com/fighters-legacy/fighters-legacy).
Implements the engine's `IContentPack` interface, using
[fx_lib](https://github.com/jomkz/fighters-codex) to transcode Jane's Fighters
Anthology (1998) asset formats into the engine's canonical open formats.

## What this is

fighters-legacy is a clean-room GPL v3 reimplementation of the combat flight simulator
engine. It has no knowledge of FA-specific formats — all content enters through
`IContentPack` as raw bytes in canonical formats (glTF, PNG, OGG Vorbis, TOML, YAML).
This repository is one implementation of that interface: it bridges a user-supplied FA
installation to the engine, transcoding each FA format on load.

**You must own a legitimate copy of Jane's Fighters Anthology to use this plugin.**
No FA game assets are included or may be committed to this repository.

## Status

**Pre-implementation.** The repository configures on all three platforms but builds no
targets yet; bridge code lands per the phase plan in [docs/roadmap.md](docs/roadmap.md).
Which FA content is already fully understood and ready to bridge — aircraft flight
models, entity stats, ~95% of 3D models, all 2D art, sound effects — is tracked in
[docs/asset-support-matrix.md](docs/asset-support-matrix.md).

## Dependencies

| Dependency | Source |
|---|---|
| fighters-legacy | Engine interface headers (`IContentPack.h`), vendored with a recorded pin (roadmap Phase 1) |
| fx_lib | `extern/fx_lib` submodule from [fighters-codex](https://github.com/jomkz/fighters-codex), pinned to a release tag |

## Quick start

```bash
git clone --recurse-submodules https://github.com/fighters-legacy/fa-content.git
cmake --preset debug
cmake --build --preset debug
```

Note: `fx_lib` does not compile on Linux/macOS until fighters-codex Phase 1 lands; the
`FA_WITH_FX_LIB` CMake option therefore defaults OFF.

See [docs/development.md](docs/development.md) for full prerequisites and build
instructions.

## License

GPL v3. See [LICENSE](LICENSE).

Final distribution positioning (free GPL plugin vs. the paid model sketched in early
engine docs) is an open decision tracked in
[#37](https://github.com/fighters-legacy/fa-content/issues/37) and
[docs/roadmap.md](docs/roadmap.md).

> **Never commit FA game assets** (`*.LIB`, `*.PIC`, `*.PAL`, `*.SH`, `*.PT`, `*.OT`,
> `*.M`, `*.MM`, `*.T2`, `*.CB8`, and the rest of the extension set in `.gitignore`).
> They are owned by Electronic Arts and are not covered by this project's license.
