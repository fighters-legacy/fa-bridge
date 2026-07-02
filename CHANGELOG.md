# Changelog

All notable changes to this project will be documented in this file.

The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).
Versioning follows [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [0.1.0] - 2026-07-02

Built against fighters-codex v0.3.0 (`extern/fx_lib`).

### Added
- `docs/roadmap.md` — charter, the 1.0 definition, seven constraint-gated phases
  (P0 Program Reset → P6 Full Coverage & 1.0) with external gates wired to live
  tracker issues in fighters-codex and fighters-legacy (#44)
- `docs/asset-support-matrix.md` — the living per-format matrix of which FA content
  the bridge can deliver today: aircraft flight models, entity definitions, ~95% of
  3D models, all 2D art, and sound effects are ready-now; missions partial; terrain,
  music, video, and campaign blocked upstream (#28)
- `extern/fx_lib` git submodule, registered and pinned to fighters-codex v0.3.0,
  with the `FA_WITH_FX_LIB` CMake option (default OFF until the codex cross-platform
  port lands) and dependabot watching the pin (#27)
- Portable stdlib-only Python release tooling: `scripts/release.py` (verifies the
  fx_lib pin is a codex release tag before tagging) and `scripts/draft-changelog.py`,
  replacing git-cliff (#29)
- `epic` label and epic issue form; milestones Phase 0–6 with epics #30–#36 (#44)

### Fixed
- Windows CI had been failing on every PR: the MSVC presets pinned the
  `Visual Studio 17 2022` generator, which `windows-latest` no longer ships (#29)
- The DCO check rejected GitHub-generated merge commits, which can never carry
  `Signed-off-by` (#43)
- The FA-asset `.gitignore` blocklist ignored non-FA extensions (`*.FLT`, `*.SHP`,
  `*.VXA`) while missing the real FA set; now cross-checked against the codex format
  docs, both case variants (#27)
- Repository links pointed at the pre-migration `jomkz` org (#29, #44)

### Changed
- **Repository renamed `fa-content` → `fa-bridge`** — it provides no content; it
  bridges the end-user's own FA installation into locally cached engine assets (#45)
- Documentation truth pass: everything previously presented as existing is now
  explicitly planned-state with its roadmap phase, and the undefined "Phase 1
  Workstream B" vocabulary is gone. `docs/architecture.md` now states the real
  engine contract — packs return raw bytes in canonical open formats past
  `AssetValidator`, so every `load*()` is a transcode (#44)
- The planned `assets/` module is renamed `transcode/` across scopes, labels, and
  issue forms — before the directory exists (#44)
- CI is truthful: the vacuous ctest step is gone and the sanitizer/coverage
  workflows are manual-only until the Phase 1 skeleton restores them with real
  tests; the release workflow extracts notes from CHANGELOG.md (#29)

[Unreleased]: https://github.com/fighters-legacy/fa-bridge/compare/v0.1.0...HEAD
[0.1.0]: https://github.com/fighters-legacy/fa-bridge/releases/tag/v0.1.0
