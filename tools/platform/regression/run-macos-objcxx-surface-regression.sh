#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../../.." && pwd)"

source "$SCRIPT_DIR/lib/common.sh"

while [[ $# -gt 0 ]]; do
    case "$1" in
        --repo-root)
            mfx_require_option_value "$1" "${2:-}"
            REPO_ROOT="${2:-}"
            shift 2
            ;;
        -h|--help)
            cat <<'USAGE'
Usage: run-macos-objcxx-surface-regression.sh [options]
  --repo-root <path>   repository root override
USAGE
            exit 0
            ;;
        *)
            mfx_fail "unknown argument: $1"
            ;;
    esac
done

CMAKE_FILE="$REPO_ROOT/MFCMouseEffect/Platform/macos/CMakeLists.txt"
if [[ ! -f "$CMAKE_FILE" ]]; then
    mfx_fail "missing macos cmake file: $CMAKE_FILE"
fi

mfx_info "repo root: $REPO_ROOT"
mfx_info "objcxx surface gate cmake: $CMAKE_FILE"

mapfile -t MM_FILES < <(find "$REPO_ROOT" -type f -name '*.mm' -print | sort)
if [[ "${#MM_FILES[@]}" -gt 0 ]]; then
    printf '[mfx:fail] unexpected .mm files detected:\n' >&2
    printf '  %s\n' "${MM_FILES[@]}" >&2
    exit 1
fi

declare -a BANNED_WILDCARD_PATTERNS=(
    '/Platform/macos/legacy/.+\\.cpp$'
    '/Platform/macos/Effects/.+\\.cpp$'
    '/Platform/macos/Overlay/.+\\.cpp$'
    '/Platform/macos/Wasm/.+\\.cpp$'
)

for pattern in "${BANNED_WILDCARD_PATTERNS[@]}"; do
    if grep -Fq "$pattern" "$CMAKE_FILE"; then
        mfx_fail "objcxx wildcard pattern still present in CMake: $pattern"
    fi
done

if ! grep -Fq 'set(MFX_MACOS_OBJCXX_SOURCE_ALLOWLIST' "$CMAKE_FILE"; then
    mfx_fail "missing explicit ObjC++ allowlist declaration in CMake"
fi
if ! grep -Fq 'if(MFX_MACOS_SOURCE IN_LIST MFX_MACOS_OBJCXX_SOURCE_ALLOWLIST)' "$CMAKE_FILE"; then
    mfx_fail "ObjC++ source selection is not using explicit allowlist"
fi

objcxx_flag_count="$(grep -c 'COMPILE_FLAGS "-x objective-c++"' "$CMAKE_FILE" || true)"
if [[ "$objcxx_flag_count" -ne 1 ]]; then
    mfx_fail "expected exactly one ObjC++ compile-flag assignment in CMake, got: $objcxx_flag_count"
fi

mapfile -t ALLOWLIST_ENTRIES < <(
    awk '
        /set\(MFX_MACOS_OBJCXX_SOURCE_ALLOWLIST/ {in_list=1; next}
        in_list && /^[[:space:]]*\)/ {in_list=0; next}
        in_list {
            line=$0
            gsub(/^[[:space:]]*"/, "", line)
            gsub(/"[[:space:]]*$/, "", line)
            if (line != "") {
                print line
            }
        }
    ' "$CMAKE_FILE"
)

if [[ "${#ALLOWLIST_ENTRIES[@]}" -eq 0 ]]; then
    mfx_info "ObjC++ allowlist is empty (all macOS sources are Swift/C++ mode)"
else
    mapfile -t DUP_ALLOWLIST < <(printf '%s\n' "${ALLOWLIST_ENTRIES[@]}" | sort | uniq -d)
    if [[ "${#DUP_ALLOWLIST[@]}" -gt 0 ]]; then
        printf '[mfx:fail] duplicated ObjC++ allowlist entries:\n' >&2
        printf '  %s\n' "${DUP_ALLOWLIST[@]}" >&2
        exit 1
    fi

    for entry in "${ALLOWLIST_ENTRIES[@]}"; do
        case "$entry" in
            '${MFX_PROJECT_ROOT}/Platform/macos/Effects/'* | \
            '${MFX_PROJECT_ROOT}/Platform/macos/Overlay/'* | \
            '${MFX_PROJECT_ROOT}/Platform/macos/Wasm/'*)
                ;;
            *)
                mfx_fail "ObjC++ allowlist entry is outside macOS target scope: $entry"
                ;;
        esac

        resolved_path="${entry//'${MFX_PROJECT_ROOT}'/$REPO_ROOT/MFCMouseEffect}"
        if [[ ! -f "$resolved_path" ]]; then
            mfx_fail "ObjC++ allowlist entry points to missing file: $entry -> $resolved_path"
        fi
    done
fi

mfx_ok "macos objcxx surface regression passed"
