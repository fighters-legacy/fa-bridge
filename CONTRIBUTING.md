# Contributing to fa-bridge

Thank you for contributing to the FA Bridge Plugin. This document covers the conventions and workflow for this repository.

For project governance and the RFC process, see [GOVERNANCE.md](GOVERNANCE.md).

---

## Commit format

This project uses [Conventional Commits](https://www.conventionalcommits.org/en/v1.0.0/).

```
<type>(<scope>): <description>

[optional body]

[optional footer(s)]
Signed-off-by: Your Name <your@email>
```

### Types

| Type | When to use |
|---|---|
| `feat` | New feature or capability |
| `fix` | Bug fix |
| `docs` | Documentation only |
| `refactor` | Code restructuring with no behaviour change |
| `perf` | Performance improvement |
| `test` | Tests only |
| `build` | Build system changes |
| `ci` | CI workflow changes |
| `chore` | Maintenance (deps, tooling) |
| `style` | Formatting, whitespace |

### Scopes

| Scope | Targets |
|---|---|
| `bridge` | `bridge/` — IContentPack implementation (created in roadmap Phase 1) |
| `transcode` | `transcode/` — canonical-format writers (created in roadmap Phase 3) |
| `build` | `CMakeLists.txt`, `CMakePresets.json`, `extern/` pins |
| `ci` | `.github/workflows/`, `.github/dependabot.yml` |
| `docs` | `docs/`, `*.md` |

### Breaking changes

Add a `BREAKING CHANGE:` footer for any change that breaks the bridge API or the asset translation interface:

```
feat(bridge)!: change IContentPack::loadMesh signature

BREAKING CHANGE: loadMesh now takes a MeshDescriptor instead of a raw path string.
```

---

## Branch naming

```
feat/bridge-mesh-loader
fix/pal-colour-index-overflow
docs/development-prerequisites
```

---

## SPDX license headers

Every new `.cpp` and `.h` file must begin with:

```cpp
// SPDX-License-Identifier: GPL-3.0-or-later
```

This is the first line of the file, before includes or any other content.

---

## Developer Certificate of Origin (DCO)

This project uses DCO instead of a CLA — the same model as the Linux kernel and Git. Every commit must carry a `Signed-off-by` line:

```
Signed-off-by: Your Name <your@email>
```

Sign off automatically with:

```bash
git commit -s
```

Or install the auto-sign hook so you never forget:

```bash
cp scripts/hooks/commit-msg .git/hooks/
chmod +x .git/hooks/commit-msg
```

The DCO certifies that you have the right to contribute the code under the project's GPL v3 license. See [developercertificate.org](https://developercertificate.org/) for the full text.

---

## Code coverage

New code added in PRs should aim for ≥70% test coverage (applies once code exists — roadmap Phase 1 onward). Codecov posts an automated comment on every PR showing the coverage delta for changed files. CI measures coverage automatically — you do not need to install anything.

---

## First-time contributor guide

1. Fork the repository on GitHub
2. Clone your fork: `git clone --recurse-submodules git@github.com:<you>/fa-bridge.git`
3. Create a branch: `git checkout -b feat/your-feature`
4. Build and verify: see [docs/development.md](docs/development.md)
5. Make your changes. Add tests where applicable.
6. Sign off: `git commit -s -m "feat(bridge): your description"`
7. Push and open a PR against `main`

---

## When to open an RFC

Open an RFC issue (using the RFC template) before starting work on:
- Changes to the `IContentPack` interface binding in this repo
- New FA format support affecting multiple modules
- New major dependencies
- Breaking changes to the bridge API

See [GOVERNANCE.md](GOVERNANCE.md) for the full RFC process.

For full build workflow and release process, see [docs/development.md](docs/development.md).
