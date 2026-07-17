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
git clone --recurse-submodules https://github.com/fighters-legacy/fa-bridge.git
cd fa-bridge
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

The build produces the plugin shared library (`libfa-bridge.so` / `fa-bridge.dll` /
`libfa-bridge.dylib`) and stages a drop-in mod directory at
`build/<preset>/stage/mods/fa-bridge/` containing the plugin and its `manifest.toml`.

### fx_lib

fx_lib is always built and linked from the `extern/fx_lib` submodule — the former
`FA_WITH_FX_LIB` option is gone (fighters-codex ships fx_lib cross-platform since its
Phase 1). Configuration fails with a clear message if the submodule is missing; fix with:

```bash
git submodule update --init --recursive
```

The codex root is embedded as a subproject: only its `lib/` (`fx::lib`, the parsers) and
`render/` (`fx::render`, unused here) build; the codex CLI, GUI, tools, and tests do not.

### `FA_BUILD_TESTS`

The test suite builds by default. Set `-DFA_BUILD_TESTS=OFF` to build only the plugin
(e.g. when packaging).

### Running tests

```bash
ctest --preset debug --output-on-failure
```

Unit tests use **Catch2 v3** (v3.7.1, the same framework and version as fighters-codex),
fetched with CMake `FetchContent` — the first configure of each build directory needs
network access. Each `TEST_CASE` registers as its own `ctest` entry. `test_plugin_load`
is deliberately a plain executable, not a Catch2 target: it rehearses the engine's
`dlopen`/`LoadLibrary` ModLoader flow in a process of its own.

---

## Install discovery and configuration

At `init()` the plugin locates the FA install through a chain — first valid
candidate wins, and a candidate must contain at least one `.LIB` file (matched
case-insensitively):

1. `FA_INSTALL_DIR` environment variable (explicit override)
2. the persisted config file — `fighters-legacy/fa-bridge/config.toml` under the
   platform config dir (`$XDG_CONFIG_HOME` or `~/.config` on Linux, `%APPDATA%`
   on Windows, `~/Library/Application Support` on macOS)
3. Windows only: a best-effort registry probe, then
   `<drive>:\JANES\Fighters Anthology` on each fixed drive

When nothing is found, the game client shows a native folder picker on first
run and persists the confirmed path; deleting the config file re-triggers the
flow. Dev/test environment knobs (each overrides the default behaviour):

| Variable | Effect |
|---|---|
| `FA_BRIDGE_CONFIG_DIR` | overrides the config directory |
| `FA_BRIDGE_CACHE_DIR` | overrides the cache directory |
| `FA_BRIDGE_NO_PROBE` | skips the registry/drive probes |

The translation cache lives under the cache directory
(`fighters-legacy/fa-bridge/` beneath `$XDG_CACHE_HOME`/`~/.cache`,
`%LOCALAPPDATA%`, or `~/Library/Caches`). It is self-invalidating against the
source archives; delete the directory at any time to clear it.

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

## Testing the plugin against an engine build

Every build stages a ready-to-load mod directory. Copy it into the `mods/` directory the
engine scans (`fl-server`: the working directory; the game: next to the binary):

```bash
cp -r build/debug/stage/mods/fa-bridge /path/to/engine/mods/
FA_INSTALL_DIR="/path/to/Janes Fighters Anthology" ./fl-server
```

Point `FA_INSTALL_DIR` at a real FA install (it must contain at least one
parseable `.LIB`) so the pack mounts and reports `Ready`. Headless hosts pass
no window, so a pack that cannot discover an install is dropped after loading
(`NeedsConfiguration` with no configure UI) — correct but noisy. In the game
client the same situation opens the first-run folder picker instead.

**Windows:** the plugin and the engine must be built in the **same configuration**
(Debug with Debug, Release with Release) — mixed CRT/iterator-debug-level builds fail
at load time. See the ABI discipline note in [architecture.md](architecture.md).

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

## Code coverage

The coverage workflow runs on every push and pull request. Locally:

```bash
cmake --preset coverage
cmake --build --preset coverage
ctest --preset coverage --output-on-failure
lcov --capture --directory . --output-file coverage.info \
     --exclude '*/_deps/*' --exclude '*/catch2/*'
lcov --remove coverage.info '/usr/*' '*/tests/*' '*/vendor/*' '*/extern/*' \
     '*/_deps/*' '*/catch2/*' --output-file coverage.info
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
