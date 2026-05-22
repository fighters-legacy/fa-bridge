# FA Content Bridge — Claude Code Instructions

## Project Overview

GPL v3. Implements `IContentPack` for [fighters-legacy](https://github.com/jomkz/fighters-legacy) using `fx_lib` from [fighters-codex](https://github.com/jomkz/fighters-codex). Translates Jane's Fighters Anthology (1998) asset formats into engine runtime types at load time.

Phase 1 Workstream B is active.

## Architecture

```
FA Install (user-supplied)
    └── fx_lib (fighters-codex submodule)
            └── IContentPack::load*() implementation (bridge/)
                    └── Engine AssetManager
```

- `bridge/` — `IContentPack` implementation; all FA-format logic lives here
- `assets/` — asset translation pipeline (texture, mesh, audio, mission data)
- `extern/fx_lib` — git submodule from fighters-codex; provides FA format parsers

**Never commit FA game assets** (`*.LIB`, `*.PIC`, `*.PAL`, `*.FLT`, `*.SHP`, `*.VXA`).

## Build

```bash
git submodule update --init --recursive   # pull fx_lib
cmake --preset debug && cmake --build --preset debug   # Linux / macOS
cmake --preset debug-msvc && cmake --build --preset debug-msvc   # Windows
```

## Conventions

- Conventional Commits with scopes: `bridge`, `assets`, `build`, `ci`, `docs`
- DCO sign-off required: `git commit -s`
<!-- REUSE-IgnoreStart -->
- SPDX identifier required on all new source files: `// SPDX-License-Identifier: GPL-3.0-or-later`
<!-- REUSE-IgnoreEnd -->
- All code must compile on Windows (MSVC), Linux (GCC/Clang), and macOS (AppleClang)

## Key files

- `README.md` — project overview and quick start
- `docs/development.md` — full build prerequisites and FA_INSTALL_DIR setup
- `docs/architecture.md` — data flow from FA install through fx_lib to engine
- `GOVERNANCE.md` — RFC process for IContentPack binding changes
