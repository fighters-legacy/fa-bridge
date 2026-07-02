#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 John McKenzie
"""Prepare a release: verify the extern/fx_lib pin, bump the version in
CMakeLists.txt, rotate CHANGELOG.md, commit both files, and create the tag.

Usage:
    python3 scripts/release.py 0.1.0

Stdlib-only; runs anywhere git and Python 3.8+ exist.
Adapted from fighters-codex scripts/release.py.
"""
import datetime
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
CMAKE = ROOT / "CMakeLists.txt"
CHANGELOG = ROOT / "CHANGELOG.md"
REPO_URL = "https://github.com/fighters-legacy/fa-content"
FX_LIB = ROOT / "extern" / "fx_lib"


def git(*args: str, cwd: Path = ROOT) -> subprocess.CompletedProcess:
    return subprocess.run(["git", "-C", str(cwd), *args],
                          capture_output=True, text=True, check=False)


def read_text(path: Path) -> tuple[str, str]:
    """Return (text, encoding) preserving a UTF-8 BOM if present."""
    raw = path.read_bytes()
    enc = "utf-8-sig" if raw.startswith(b"\xef\xbb\xbf") else "utf-8"
    return raw.decode(enc), enc


def fail(msg: str) -> int:
    print(msg, file=sys.stderr)
    return 1


def fx_lib_pin() -> str:
    """Return the fighters-codex release tag extern/fx_lib is pinned to.

    Releases must always be built against a tagged fighters-codex release,
    never an arbitrary commit — the pin is part of the release notes.
    """
    if not (FX_LIB / ".git").exists():
        raise RuntimeError(
            "extern/fx_lib is not initialized. "
            "Run: git submodule update --init --recursive")
    r = git("describe", "--tags", "--exact-match", "HEAD", cwd=FX_LIB)
    if r.returncode != 0:
        sha = git("rev-parse", "--short", "HEAD", cwd=FX_LIB).stdout.strip()
        raise RuntimeError(
            f"extern/fx_lib is pinned to {sha}, which is not a fighters-codex "
            "release tag. Bump the submodule to a tagged release first.")
    return r.stdout.strip()


def main() -> int:
    if len(sys.argv) != 2 or not re.fullmatch(r"\d+\.\d+\.\d+", sys.argv[1]):
        return fail("Usage: release.py X.Y.Z  (e.g. release.py 0.1.0)")
    version = sys.argv[1]
    tag = f"v{version}"

    if git("status", "--porcelain", "--untracked-files=no").stdout.strip():
        return fail("Working tree has uncommitted changes. "
                    "Commit or stash them before releasing.")
    if git("tag", "--list", tag).stdout.strip():
        return fail(f"Tag '{tag}' already exists.")

    try:
        codex_tag = fx_lib_pin()
    except RuntimeError as e:
        return fail(str(e))

    today = datetime.date.today().isoformat()

    # --- CMakeLists.txt ---
    cmake, cmake_enc = read_text(CMAKE)
    new_cmake, n = re.subn(r"VERSION \d+\.\d+\.\d+", f"VERSION {version}",
                           cmake, count=1)
    if n == 0:
        return fail("Could not find 'VERSION x.y.z' in CMakeLists.txt")
    CMAKE.write_bytes(new_cmake.encode(cmake_enc))

    # --- CHANGELOG.md ---
    chlog, chlog_enc = read_text(CHANGELOG)
    pin_note = f"Built against fighters-codex {codex_tag} (`extern/fx_lib`)."
    new_chlog, n = re.subn(
        r"## \[Unreleased\]",
        f"## [Unreleased]\n\n## [{version}] - {today}\n\n{pin_note}",
        chlog, count=1)
    if n == 0:
        return fail("CHANGELOG.md does not contain an '## [Unreleased]' section.")
    new_chlog = re.sub(r"\[Unreleased\]: .+",
                       f"[Unreleased]: {REPO_URL}/compare/{tag}...HEAD",
                       new_chlog, count=1)
    # Insert the new release link before the first existing version link only.
    new_chlog = re.sub(r"(?m)^(\[\d+\.\d+\.\d+\]:)",
                       f"[{version}]: {REPO_URL}/releases/tag/{tag}\n\\1",
                       new_chlog, count=1)
    if f"[{version}]:" not in new_chlog:
        new_chlog = new_chlog.rstrip("\n") + \
            f"\n[{version}]: {REPO_URL}/releases/tag/{tag}\n"
    CHANGELOG.write_bytes(new_chlog.encode(chlog_enc))

    # --- Commit and tag (DCO sign-off required in this repo) ---
    for cmd in (["add", str(CMAKE), str(CHANGELOG)],
                ["commit", "-s", "-m", f"chore: release {tag}"],
                ["tag", tag]):
        r = git(*cmd)
        if r.returncode != 0:
            return fail(r.stderr.strip() or f"git {cmd[0]} failed")

    print(f"""
Release {tag} prepared (against fighters-codex {codex_tag}).
Review the commit, then push:
  git push origin main --tags

Post-release checklist:
  - Verify the Release workflow published the release notes
  - Update docs/asset-support-matrix.md rows if this release shifted any
""")
    return 0


if __name__ == "__main__":
    sys.exit(main())
