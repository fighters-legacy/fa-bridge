# Security Policy

## Supported versions

| Version | Supported |
|---|---|
| Unreleased (pre-1.0) | Yes |

## Reporting a vulnerability

**Do NOT open a public GitHub issue for security vulnerabilities.**

Email `fighters-legacy@mkz.io` with:
- A description of the vulnerability
- Steps to reproduce
- Potential impact
- Any suggested mitigations

**Response timeline:**
- 48 hours: acknowledgement
- 7 days: initial assessment and severity classification
- Coordinated disclosure once a fix is available

## Scope

This repository will ship a **native engine plugin that parses untrusted binary input**
(FA asset files); malformed-input handling is squarely in scope. In scope once code
exists (roadmap Phase 1 onward):

- Memory corruption in the bridge or transcode layers when fed malformed FA files
  (`*.LIB`, `*.PIC`, `*.PAL`, `*.SH`, `*.PT`, `*.T2`, …)
- Path traversal or arbitrary file access via asset loading or the FA-install mount layer
- Anything that undermines the engine's plugin trust surface (manifest trust levels, the
  native-code consent flow)

Out of scope:
- Vulnerabilities in the `fx_lib` parsers themselves — report to
  [fighters-codex](https://github.com/jomkz/fighters-codex/security) (this repo bumps the
  submodule pin when the fix lands)
- Vulnerabilities in the fighters-legacy engine core — report to the
  [fighters-legacy repo](https://github.com/fighters-legacy/fighters-legacy/security)
- Issues requiring possession of a modified or pirated FA installation
