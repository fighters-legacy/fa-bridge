# Engine interface header pin

Verbatim copies of the fighters-legacy engine's content-pack interface headers
(`content/`) and the window interface the configure() flow calls (`platform/`).
Vendored because the engine exports no CMake package; the set is closed and
std-only. **This pin is load-bearing** — the `IContentPack` interface may still
gain pure virtuals, and a stale pin means the plugin compiles against a contract
the engine no longer honours. Bump deliberately, never casually.

## Current pin

| | |
|---|---|
| Source | [fighters-legacy/fighters-legacy](https://github.com/fighters-legacy/fighters-legacy) `engine/content/` + `platform/` |
| Tag | `v0.3.6` |
| Commit | `2ed613f4bae218941df2ad6ece17a10b151e4fc9` |
| Vendored | 2026-07-17 |

The `platform/` window headers (`IWindow.h` plus its includes `IDisplay.h`,
`IWindowEventHandler.h`) are vendored so `configure()` can call
`IWindow::showFolderDialog` / `showMessageBox` in the first-run flow
(engine#665). **ABI note:** `showFolderDialog` is a virtual — the vtable layout
ties the plugin to the engine header revision, so engine and plugin must be
rebuilt together across any `IWindow` change; that is exactly what this pin
disciplines.

## Files

```
a89404aecdbbe6a5f2c1466d034b15ef069d688edd274579d60718acc63d4da4  content/AssetTypes.h
94a12a02add00a456bda82de2f2baae3cab2eff91b4c8a15466d23c4d36bd471  content/IContentPack.h
02ef07e5c918ea449b0054999d02fd2a343058059b211cbdbbce9945f6261a2f  content/TrustLevel.h
58abc93af173e78e4916c9a6d77911dc234fedc3a352038517be80d751fc3522  platform/IDisplay.h
b0a5abe6f9984e8e4c88d2513024aafa96f4fd4b7b57a1dad4bfff07d944e517  platform/IWindow.h
8537e1eefaf5b0c06facdc581bec3607c4b25286f87a0013414cc2194aa869ec  platform/IWindowEventHandler.h
```

Verify integrity (from `extern/fl-headers/`):

```bash
grep -E '^[0-9a-f]{64}' PIN.md | sha256sum -c -
```

Verify against an engine checkout:

```bash
for h in IContentPack.h AssetTypes.h TrustLevel.h; do
  git -C ../../../fighters-legacy show "v0.3.6:engine/content/$h" \
    | diff - "content/$h" && echo "content/$h OK"
done
for h in IWindow.h IDisplay.h IWindowEventHandler.h; do
  git -C ../../../fighters-legacy show "v0.3.6:platform/$h" \
    | diff - "platform/$h" && echo "platform/$h OK"
done
```

## Update procedure

1. Pick the new engine tag; copy the headers verbatim — `engine/content/` →
   `content/`, `platform/` → `platform/` — at that tag
   (`git show <tag>:<path>`). Never edit the copies — not even the SPDX lines.
2. `diff` the old and new copies. **If `IContentPack` gained pure virtuals,
   implement them in `FaContentPack` in the same PR** — the plugin must never
   ship compiled against a narrower interface than the engine calls.
3. Regenerate the sha256 list above (`sha256sum content/*.h platform/*.h`),
   update the pin table (tag, commit, dates), and update `manifest.toml.in`'s
   `engine-api` if the engine's major API version changed.

## No automation watches this pin

Dependabot watches `extern/fx_lib` (a git submodule); these are plain files —
updates are manual and deliberate, by design.

## Licensing

The headers are GPL-3.0-or-later, © MKZ Systems LLC (see `REUSE.toml`).
`IContentPack.h` carries the engine's linking exception for content-pack
implementors in its header comment; the exception has no SPDX identifier, so
the in-file text is authoritative.
