#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd -- "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd -- "$script_dir/../../.." && pwd)"

usage() {
    cat <<'EOF'
Usage:
  tools/platform/build/build-windows-project.sh [options]

Options:
  --configuration <Release|Shipping>  Build configuration (default: Release)
  --gpu                               Enable Windows GPU build
  --no-gpu                            Disable Windows GPU build (default)
  -h, --help                          Show this help
EOF
}

detect_windows_host() {
    case "$(uname -s)" in
    MINGW*|MSYS*|CYGWIN*)
        return 0
        ;;
    *)
        return 1
        ;;
    esac
}

require_windows_host() {
    if ! detect_windows_host; then
        echo "this Windows build script is Windows-only" >&2
        exit 1
    fi
}

find_msbuild() {
    local candidates=(
        "/c/Program Files/Microsoft Visual Studio/18/Professional/MSBuild/Current/Bin/amd64/MSBuild.exe"
        "/c/Program Files/Microsoft Visual Studio/18/Professional/MSBuild/Current/Bin/MSBuild.exe"
        "/c/Program Files/Microsoft Visual Studio/18/Insiders/MSBuild/Current/Bin/amd64/MSBuild.exe"
        "/c/Program Files/Microsoft Visual Studio/18/Insiders/MSBuild/Current/Bin/MSBuild.exe"
    )
    local candidate
    for candidate in "${candidates[@]}"; do
        if [[ -x "$candidate" ]]; then
            printf '%s\n' "$candidate"
            return 0
        fi
    done
    echo "MSBuild.exe not found in expected VS2026 locations" >&2
    exit 1
}

configuration="Release"
enable_windows_gpu_effects=false

while [[ $# -gt 0 ]]; do
    case "$1" in
    --configuration)
        [[ $# -ge 2 ]] || { echo "missing value for --configuration" >&2; exit 1; }
        configuration="$2"
        shift 2
        ;;
    --gpu)
        enable_windows_gpu_effects=true
        shift
        ;;
    --no-gpu)
        enable_windows_gpu_effects=false
        shift
        ;;
    -h|--help)
        usage
        exit 0
        ;;
    *)
        echo "unknown argument: $1" >&2
        exit 1
        ;;
    esac
done

case "$configuration" in
Release|Shipping)
    ;;
*)
    echo "unsupported Windows configuration: $configuration" >&2
    exit 1
    ;;
esac

require_windows_host

project_path="$repo_root/MFCMouseEffect/MFCMouseEffect.vcxproj"
if [[ ! -f "$project_path" ]]; then
    echo "missing project file: $project_path" >&2
    exit 1
fi

msbuild_exe="$(find_msbuild)"
MSYS2_ARG_CONV_EXCL='*' "$msbuild_exe" \
    "$(cygpath -w "$project_path")" \
    /t:Build \
    "/p:Configuration=$configuration" \
    /p:Platform=x64 \
    "/p:MfxEnableWindowsGpuEffects=$enable_windows_gpu_effects" \
    /nologo \
    /v:minimal
