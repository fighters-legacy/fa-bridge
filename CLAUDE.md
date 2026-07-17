# FA Content Bridge — Claude Code Instructions

## What this project is

The FA bridge plugin for [fighters-legacy](https://github.com/fighters-legacy/fighters-legacy):
a native `IContentPack` implementation that **transcodes** Jane's Fighters Anthology (1998)
assets from a user-supplied install into the engine's canonical open formats at load time.
**The working plugin is the primary output.** Reverse-engineering knowledge is consumed
from fighters-codex (docs + `fx_lib`), never produced here — format gaps become
fighters-codex issues.

The engine contract: content packs return **raw bytes in canonical formats** — glTF `.glb`
(meshes), PNG/KTX2 (textures), OGG Vorbis (audio), TOML (flight models, entity defs), YAML
(missions), Lua source (AI). The engine's `AssetValidator` rejects anything else, so every
`load*()` is a transcode, not a pass-through.

Work is planned in [docs/roadmap.md](docs/roadmap.md): milestones = phases, `epic`-labeled
issues + native sub-issues = work breakdown. What can be bridged today lives in
[docs/asset-support-matrix.md](docs/asset-support-matrix.md). No standalone TODO files.

## Related repositories

- **[fighters-legacy/fighters-legacy](https://github.com/fighters-legacy/fighters-legacy)** —
  the engine; owns `IContentPack`. Its three interface headers are vendored at
  `extern/fl-headers` with the pin recorded in `extern/fl-headers/PIN.md`; the interface
  may gain pure virtuals, so bump the pin deliberately, never casually.
- **[jomkz/fighters-codex](https://github.com/jomkz/fighters-codex)** (MIT) — FA
  reverse-engineering docs and `fx_lib`, consumed as the `extern/fx_lib` submodule pinned
  to a release tag. RE documentation lives *there*; implementations live *here*.
- **License boundary:** codex is MIT (one-way consumption is fine); this repo is
  GPL-3.0-or-later. Final distribution positioning is an open owner decision
  ([#37](https://github.com/fighters-legacy/fa-bridge/issues/37)).

## Environments

- **Primary development: Fedora Linux.** `cmake --preset debug && cmake --build --preset debug`.
- **fx_lib is linked unconditionally** (no build option): the codex root embeds as a
  subproject building only `fx::lib` + `fx::render`. Upstream supports Linux + Windows;
  our macOS CI leg is the proving ground for codex#155.
- The licensed FA install lives on the fighters-codex **Windows bench**; runtime testing
  here uses `FA_INSTALL_DIR` (required at runtime only, never at build time). Test/dev
  env knobs: `FA_BRIDGE_CONFIG_DIR`, `FA_BRIDGE_CACHE_DIR`, `FA_BRIDGE_NO_PROBE`
  (docs/development.md — tests set them for hermeticity; never rely on a machine's real
  persisted config in tests).
- **Never commit FA game assets.** The `.gitignore` blocks the real FA extension set
  (`*.LIB`, `*.PIC`, `*.PAL`, `*.SH`, `*.PT`, `*.OT`, `*.M`, `*.MM`, `*.T2`, `*.CB8`, …);
  FA files with generic extensions (`.TXT`, `.BIN`, `.DAT`, `.CFG`) can't be
  blanket-ignored and must be caught in review.

## Conventions

- Conventional Commits with scopes per [CONTRIBUTING.md](CONTRIBUTING.md):
  `bridge`, `transcode`, `build`, `ci`, `docs`. Branches `<type>/<kebab-description>`.
- DCO sign-off required: `git commit -s`.
<!-- REUSE-IgnoreStart -->
- SPDX identifier required on all new source files: `// SPDX-License-Identifier: GPL-3.0-or-later`
<!-- REUSE-IgnoreEnd -->
- C++20; all code must compile on Windows (MSVC), Linux (GCC/Clang), and macOS
  (AppleClang). Plugin ABI: same toolchain family and CRT config as the engine.
- Releases: `python3 scripts/draft-changelog.py` to draft, curate CHANGELOG.md, then
  `python3 scripts/release.py X.Y.Z` and push with tags. The script refuses to release
  unless `extern/fx_lib` is pinned to a codex release tag.

## The docs-currency rule

Every change that shifts what the bridge can do updates, **in the same PR**:
[docs/asset-support-matrix.md](docs/asset-support-matrix.md) (the affected row) and the
relevant reference doc. The truth-pass lesson: claims that aren't mechanically checked
drift — don't add unchecked claims to README.

## Key files

- `README.md` — project overview and status
- `docs/roadmap.md` — phases, gates, and the 1.0 definition
- `docs/asset-support-matrix.md` — what can be bridged today, and what blocks the rest
- `docs/development.md` — build prerequisites and FA_INSTALL_DIR setup
- `docs/architecture.md` — architecture: FA install → fx_lib → transcode → engine
- `GOVERNANCE.md` — RFC process for IContentPack binding changes
