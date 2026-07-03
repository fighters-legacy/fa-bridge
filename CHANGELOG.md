# Changelog

All notable changes to this project will be documented in this file.

The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).
Versioning follows [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [0.2.1] - 2026-07-02

Built against fighters-codex v0.3.0 (`extern/fx_lib`).

### Fixed
- The release workflow published releases with empty notes and a failing
  provenance step: #26's action-gh-release v3 bump had rewired the notes body
  to a nonexistent git-cliff step and added a broken attestation step. Notes
  wiring restored; attestation removed until artifact packaging lands (#51).
  v0.2.0's published notes were repaired by hand (#50)

## [0.2.0] - 2026-07-02

Built against fighters-codex v0.3.0 (`extern/fx_lib`).

### Added
- Plugin skeleton (roadmap Phase 1, #31): `bridge/` builds the plugin shared
  library (`libfa-bridge.so` / `fa-bridge.dll` / `libfa-bridge.dylib`) with a
  stub `fa::FaContentPack` implementing the engine's `IContentPack` —
  `priority` 100 (overrides fl-base-pack), `init()` keyed on `FA_INSTALL_DIR`,
  every `load*()` empty until the transcode pipelines land — and exports
  `fighters_legacy_create_pack()` for the engine's `ModLoader` (#10)
- `extern/fl-headers/` — the engine's three interface headers vendored verbatim
  at engine v0.2.6, with `PIN.md` recording tag/SHA, per-file sha256 hashes, and
  the update procedure
- `packaging/manifest.toml.in` + build-time staging: every build produces a
  drop-in mod directory at `build/<preset>/stage/mods/fa-bridge/`
- Assert-based smoke tests: `test_fa_content_pack` (stub contract, all `init()`
  states) and `test_plugin_load` (loads the built plugin exactly as the
  engine's `ModLoader` does — dlopen/LoadLibrary, factory symbol, manifest
  consistency)
- `FA_BUILD_TESTS` CMake option (default ON); MSVC test presets; `ctest` step
  restored in CI with `noTestsAction: error`; sanitizer and coverage workflows
  restored to run on every push and pull request
- `.github/workflows/add-to-project.yml` — auto-adds newly opened (or reopened)
  issues and pull requests to the org-wide
  [Fighters Ecosystem project board](https://github.com/orgs/fighters-legacy/projects/3)
  via `actions/add-to-project`; uses `pull_request_target` so fork PRs can read
  the `ADD_TO_PROJECT_PAT` org secret without checking out untrusted code

### Fixed
- `asan` and `coverage` presets now pass their instrumentation flags to shared
  library links (`CMAKE_SHARED_LINKER_FLAGS`) — required for a SHARED plugin
- `debug-msvc` / `release-msvc` test presets exist, so the
  `ctest --preset debug-msvc` instruction in docs/development.md works

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

[Unreleased]: https://github.com/fighters-legacy/fa-bridge/compare/v0.2.1...HEAD
[0.2.1]: https://github.com/fighters-legacy/fa-bridge/releases/tag/v0.2.1
[0.2.0]: https://github.com/fighters-legacy/fa-bridge/releases/tag/v0.2.0
[0.1.0]: https://github.com/fighters-legacy/fa-bridge/releases/tag/v0.1.0
