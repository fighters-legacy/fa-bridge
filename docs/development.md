# Development Guide

## Prerequisites

### Linux — Fedora (primary maintainer platform)

```bash
sudo dnf install cmake ninja-build gcc g++ clang clang-tools-extra lcov
```

### Linux — Ubuntu/Debian

```bash
sudo apt-get update
sudo apt-get install -y cmake ninja-build gcc g++ clang clang-format lcov
```

### Windows (MSVC)

1. Install [Visual Studio](https://visualstudio.microsoft.com/) with the **Desktop development with C++** workload
2. Optional: install Ninja via `winget install Ninja-build.Ninja`

### macOS (Apple Silicon, 13+)

```bash
xcode-select --install
brew install cmake ninja
```

For platform-specific fighters-legacy prerequisites (Vulkan SDK, SDL3, OpenAL Soft), see [fighters-legacy/docs/development.md](https://github.com/fighters-legacy/fighters-legacy/blob/main/docs/development.md).

---

## First-time setup

```bash
git clone --recurse-submodules https://github.com/fighters-legacy/fa-content.git
cd fa-content
```

If you already cloned without `--recurse-submodules`:

```bash
git submodule update --init --recursive
```

This pulls `extern/fx_lib` from fighters-codex — the FA format parser library, pinned to a codex release tag.

---

## Building

```bash
# Linux / macOS
cmake --preset debug
cmake --build --preset debug

# Windows (PowerShell)
cmake --preset debug-msvc
cmake --build --preset debug-msvc
```

**Current status:** the project configures on all three platforms but defines no build
targets yet — bridge code lands per [roadmap.md](roadmap.md) Phase 1.

### `FA_WITH_FX_LIB`

fx_lib does not compile on Linux/macOS until fighters-codex Phase 1 lands, so the
`FA_WITH_FX_LIB` option defaults **OFF**. To build against fx_lib (e.g. on Windows, or
once the codex port lands):

```bash
cmake --preset debug -DFA_WITH_FX_LIB=ON
```

### Running tests (from Phase 1 onward)

There is no `tests/` tree yet; it arrives with the Phase 1 plugin skeleton. From then on:

```bash
ctest --preset debug --output-on-failure
```

---

## Runtime testing against a real FA installation (from Phase 2 onward)

Set `FA_INSTALL_DIR` to point at your FA installation directory before running integration tests:

```bash
export FA_INSTALL_DIR="/path/to/Janes Fighters Anthology"
ctest --preset debug --output-on-failure
```

On Windows (PowerShell):

```powershell
$env:FA_INSTALL_DIR = "C:\Games\Janes Fighters Anthology"
ctest --preset debug-msvc --output-on-failure
```

Tests that require a real FA installation are skipped if `FA_INSTALL_DIR` is not set. CI runs without it; only the unit and integration tests that operate on bundled synthetic fixtures run in CI.

---

## Git setup

Install the commit-msg hook to auto-append `Signed-off-by` (DCO requirement):

```bash
cp scripts/hooks/commit-msg .git/hooks/
chmod +x .git/hooks/commit-msg
```

---

## Local preset overrides

Create `CMakeUserPresets.json` in the repo root to override preset defaults (e.g. a different build directory or custom toolchain). This file is gitignored.

---

## Code coverage (from Phase 1 onward)

The coverage workflow is manual-only until there is code to measure. Locally:

```bash
cmake --preset coverage
cmake --build --preset coverage
ctest --preset coverage --output-on-failure
lcov --capture --directory . --output-file coverage.info
lcov --remove coverage.info '/usr/*' '*/tests/*' '*/vendor/*' \
     --output-file coverage.info
```

---

## Release workflow

Releases are tagged with `vMAJOR.MINOR.PATCH`. Both scripts are stdlib-only Python — no extra tooling to install.

```bash
python3 scripts/draft-changelog.py     # draft [Unreleased] entries from conventional commits
# curate CHANGELOG.md by hand
python3 scripts/release.py X.Y.Z       # verify fx_lib pin, bump version, rotate changelog, commit + tag
git push origin main --tags
```

`release.py` refuses to release unless `extern/fx_lib` is pinned to a fighters-codex
release tag, and writes that pin into the release notes.
