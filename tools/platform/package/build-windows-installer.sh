#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd -- "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd -- "$script_dir/../../.." && pwd)"

usage() {
    cat <<'EOF'
Usage:
  tools/platform/package/build-windows-installer.sh [options]

Options:
  --output-dir <path>     Output directory for installer (default: Install/windows)
  --package-name <name>   Installer base filename without .exe (default: script default)
  --skip-build            Skip rebuilding Windows Release|x64 project
  --skip-webui-build      Accepted for CLI parity; currently no-op on Windows
  --gpu                   Build/package the Windows GPU runtime
  --no-gpu                Exclude Windows GPU runtime and do not package webgpu_dawn.dll
  -h, --help              Show this help
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
        echo "this packaging script is Windows-only" >&2
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

find_iscc() {
    local candidates=(
        "/d/inno_setup/Inno Setup 6/ISCC.exe"
        "/c/Program Files (x86)/Inno Setup 6/ISCC.exe"
        "/c/Program Files/Inno Setup 6/ISCC.exe"
    )
    local candidate
    for candidate in "${candidates[@]}"; do
        if [[ -x "$candidate" ]]; then
            printf '%s\n' "$candidate"
            return 0
        fi
    done

    local from_path=""
    if from_path="$(command -v ISCC.exe 2>/dev/null)"; then
        printf '%s\n' "$from_path"
        return 0
    fi

    echo "ISCC.exe not found in expected locations or PATH" >&2
    exit 1
}

output_dir="$repo_root/Install/windows"
package_name=""
skip_build=0
skip_webui_build=0
enable_windows_gpu_effects=false

while [[ $# -gt 0 ]]; do
    case "$1" in
    --output-dir)
        [[ $# -ge 2 ]] || { echo "missing value for --output-dir" >&2; exit 1; }
        output_dir="$2"
        shift 2
        ;;
    --package-name)
        [[ $# -ge 2 ]] || { echo "missing value for --package-name" >&2; exit 1; }
        package_name="$2"
        shift 2
        ;;
    --skip-build)
        skip_build=1
        shift
        ;;
    --skip-webui-build)
        skip_webui_build=1
        shift
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

require_windows_host

if [[ "$skip_webui_build" == "1" ]]; then
    echo "[info] --skip-webui-build is currently a no-op on Windows packaging; using existing generated WebUI assets" >&2
fi

project_path="$repo_root/MFCMouseEffect/MFCMouseEffect.vcxproj"
iss_script_path="$repo_root/Install/MFCMouseEffect.iss"

if [[ ! -f "$project_path" ]]; then
    echo "missing project file: $project_path" >&2
    exit 1
fi
if [[ ! -f "$iss_script_path" ]]; then
    echo "missing installer script: $iss_script_path" >&2
    exit 1
fi

mkdir -p "$output_dir"

if [[ "$skip_build" != "1" ]]; then
    msbuild_exe="$(find_msbuild)"
    MSYS2_ARG_CONV_EXCL='*' "$msbuild_exe" \
        "$(cygpath -w "$project_path")" \
        /t:Build \
        /p:Configuration=Release \
        /p:Platform=x64 \
        "/p:MfxEnableWindowsGpuEffects=$enable_windows_gpu_effects" \
        /nologo \
        /v:minimal
fi

iscc_exe="$(find_iscc)"

iscc_args=(
    "/O$(cygpath -w "$output_dir")"
    "/DMfxEnableWindowsGpuEffects=$enable_windows_gpu_effects"
)
if [[ -n "$package_name" ]]; then
    iscc_args+=("/F$package_name")
fi
iscc_args+=("$(cygpath -w "$iss_script_path")")

MSYS2_ARG_CONV_EXCL='*' "$iscc_exe" "${iscc_args[@]}"

if [[ -n "$package_name" ]]; then
    printf 'installer=%s/%s.exe\n' "$output_dir" "$package_name"
fi
