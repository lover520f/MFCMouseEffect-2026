#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

TARGET_REPO="$REPO_ROOT"

while [[ $# -gt 0 ]]; do
    case "$1" in
        --repo-root)
            TARGET_REPO="${2:-}"
            shift 2
            ;;
        -h|--help)
            cat <<'USAGE'
Usage: check-no-objcxx-edits.sh [--repo-root <path>]
Fail when tracked/staged/untracked edits include .mm or .m files.
Set MFX_ALLOW_OBJCXX_EDITS=1 to bypass.
USAGE
            exit 0
            ;;
        *)
            echo "[mfx:fail] unknown argument: $1" >&2
            exit 1
            ;;
    esac
done

if [[ "${MFX_ALLOW_OBJCXX_EDITS:-0}" == "1" ]]; then
    echo "[mfx:info] objcxx edit guard bypassed (MFX_ALLOW_OBJCXX_EDITS=1)"
    exit 0
fi

if [[ ! -d "$TARGET_REPO/.git" ]]; then
    echo "[mfx:fail] repo root does not contain .git: $TARGET_REPO" >&2
    exit 1
fi

cd "$TARGET_REPO"

mapfile -t changed_paths < <(git status --porcelain | awk '{print $NF}')
if [[ "${#changed_paths[@]}" -eq 0 ]]; then
    echo "[mfx:ok] objcxx edit guard passed (no workspace changes)"
    exit 0
fi

violations=()
for path in "${changed_paths[@]}"; do
    case "$path" in
        *.mm|*.m)
            violations+=("$path")
            ;;
    esac
done

if [[ "${#violations[@]}" -gt 0 ]]; then
    echo "[mfx:fail] objcxx edits are disallowed by current closure policy"
    for path in "${violations[@]}"; do
        echo "  - $path"
    done
    echo "[mfx:info] set MFX_ALLOW_OBJCXX_EDITS=1 only with explicit user approval"
    exit 1
fi

echo "[mfx:ok] objcxx edit guard passed"
