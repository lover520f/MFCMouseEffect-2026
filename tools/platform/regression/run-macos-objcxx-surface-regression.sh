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

if grep -Fq 'set(MFX_MACOS_OBJCXX_SOURCE_ALLOWLIST' "$CMAKE_FILE"; then
    mfx_fail "obsolete ObjC++ allowlist block is still present in CMake"
fi
if grep -Fq 'if(MFX_MACOS_SOURCE IN_LIST MFX_MACOS_OBJCXX_SOURCE_ALLOWLIST)' "$CMAKE_FILE"; then
    mfx_fail "obsolete ObjC++ allowlist source loop is still present in CMake"
fi

if grep -Fq 'COMPILE_FLAGS "-x objective-c++"' "$CMAKE_FILE"; then
    mfx_fail "obsolete ObjC++ compile flag assignment is still present in CMake"
fi

if grep -Eq 'objective-c\+\+|OBJCXX|LANGUAGE[[:space:]]+OBJCXX' "$CMAKE_FILE"; then
    mfx_fail "CMake still contains Objective-C++ specific compile rules"
fi

declare -a COMPILE_COMMANDS_CANDIDATES=(
    "/tmp/mfx-platform-macos-build/compile_commands.json"
    "/tmp/mfx-platform-macos-core-build/compile_commands.json"
    "/tmp/mfx-platform-macos-core-automation-build/compile_commands.json"
)
compile_commands_checked=0
for compile_commands_file in "${COMPILE_COMMANDS_CANDIDATES[@]}"; do
    if [[ ! -f "$compile_commands_file" ]]; then
        continue
    fi
    compile_commands_checked=$((compile_commands_checked + 1))
    mfx_info "objcxx surface gate compile_commands: $compile_commands_file"
    if grep -Eq -- 'objective-c\+\+|OBJCXX|-x[[:space:]]+objective-c\+\+' "$compile_commands_file"; then
        mfx_fail "compile_commands still contains Objective-C++ compile flags: $compile_commands_file"
    fi
done
if [[ "$compile_commands_checked" -eq 0 ]]; then
    mfx_info "objcxx surface gate compile_commands: none found, CMake-only checks applied"
fi

mfx_ok "macos objcxx surface regression passed"
