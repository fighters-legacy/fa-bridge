# Governance

fa-content follows the same governance model as [fighters-legacy](https://github.com/fighters-legacy/fighters-legacy). This document summarises how it applies to this repository.

## Project Lead (BDFL)

**@jomkz** is the project lead (Benevolent Dictator for Life). Responsibilities: merges to `main`, committer status grants, final say on disputes, CoC enforcement.

## Committers

Committer status is earned after 5+ quality merged PRs. Committers are initially scoped to the modules they contribute to (`bridge` or `transcode`, once those modules exist — see [docs/roadmap.md](docs/roadmap.md)). Multi-module sustained contribution leads to global committer status. Twelve months of inactivity moves a committer to emeritus status.

## Decision making

**Lazy consensus:** ordinary changes (bug fixes, new features, refactors) land after 72 hours with no objections from committers.

**RFC process:** required for changes to the `IContentPack` interface binding, new FA format support affecting multiple modules, major new dependencies, and breaking changes to the bridge API. Minimum 14-day discussion period. Decisions are recorded in the RFC issue using `rfc` + `status:` labels.

For guidance on when to open an RFC, see [CONTRIBUTING.md](CONTRIBUTING.md).

## Code of Conduct

Enforcement by the project lead. Appeals to `fighters-legacy@mkz.io`.

See [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md).
