#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd -- "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd -- "$script_dir/../../.." && pwd)"

usage() {
    cat <<'EOF'
Usage:
  tools/platform/manual/run-webui-dev-manual.sh [options]

Options:
  --workspace-dir <path>      WebUIWorkspace directory (default: <repo>/MFCMouseEffect/WebUIWorkspace)
  --probe-file <path>         Probe file path (default: /tmp/mfx-core-websettings.probe)
  --host <host>               Preferred Vite host (default: 127.0.0.1)
  --no-open                   Do not open browser automatically
  -h, --help                  Show this help
EOF
}

workspace_dir="$repo_root/MFCMouseEffect/WebUIWorkspace"
probe_file="/tmp/mfx-core-websettings.probe"
preferred_host="127.0.0.1"
open_browser=1

while [[ $# -gt 0 ]]; do
    case "$1" in
    --workspace-dir)
        [[ $# -ge 2 ]] || { echo "missing value for --workspace-dir" >&2; exit 2; }
        workspace_dir="$2"
        shift 2
        ;;
    --probe-file)
        [[ $# -ge 2 ]] || { echo "missing value for --probe-file" >&2; exit 2; }
        probe_file="$2"
        shift 2
        ;;
    --host)
        [[ $# -ge 2 ]] || { echo "missing value for --host" >&2; exit 2; }
        preferred_host="$2"
        shift 2
        ;;
    --no-open)
        open_browser=0
        shift
        ;;
    -h|--help)
        usage
        exit 0
        ;;
    *)
        echo "unknown argument: $1" >&2
        exit 2
        ;;
    esac
done

require_cmd() {
    if ! command -v "$1" >/dev/null 2>&1; then
        echo "required command missing: $1" >&2
        exit 1
    fi
}

trim_text() {
    printf '%s' "${1:-}" | awk '{$1=$1; print}'
}

probe_value() {
    local key="$1"
    local file_path="$2"
    sed -n "s/^${key}=//p" "$file_path" | head -n 1
}

if [[ ! -d "$workspace_dir" ]]; then
    echo "workspace directory missing: $workspace_dir" >&2
    exit 1
fi

require_cmd pnpm
require_cmd curl
require_cmd sed
require_cmd awk

state_file="/tmp/mfx-webui-dev.state"
log_file="/tmp/mfx-webui-dev.log"

extract_url_from_log() {
    local file_path="$1"
    if [[ ! -f "$file_path" ]]; then
        return 1
    fi
    sed -nE 's/.*Local:[[:space:]]*(https?:\/\/[^[:space:]]+).*/\1/p' "$file_path" | tail -n 1
}

url_alive() {
    local url="$1"
    [[ -n "$url" ]] || return 1
    curl -fsS --max-time 1 "${url}__mfx/dev-runtime" >/dev/null 2>&1
}

current_pid=""
current_url=""
if [[ -f "$state_file" ]]; then
    current_pid="$(trim_text "$(probe_value pid "$state_file")")"
    current_url="$(trim_text "$(probe_value url "$state_file")")"
fi

reuse_existing=0
if [[ -n "$current_pid" ]] && kill -0 "$current_pid" 2>/dev/null && url_alive "$current_url"; then
    reuse_existing=1
else
    rm -f "$state_file"
fi

if [[ "$reuse_existing" != "1" ]]; then
    : >"$log_file"
    (
        cd "$workspace_dir"
        nohup pnpm run dev -- --host "$preferred_host" >"$log_file" 2>&1 &
        echo "$!" > "$state_file.pid"
    )
    current_pid="$(cat "$state_file.pid")"
    rm -f "$state_file.pid"

    current_url=""
    for _attempt in $(seq 1 120); do
        current_url="$(trim_text "$(extract_url_from_log "$log_file")")"
        if [[ -n "$current_url" ]] && url_alive "$current_url"; then
            break
        fi
        sleep 0.25
    done

    if [[ -z "$current_url" ]]; then
        echo "failed to discover Vite dev URL; see $log_file" >&2
        exit 1
    fi

    cat >"$state_file" <<EOF
pid=$current_pid
url=$current_url
log_file=$log_file
workspace_dir=$workspace_dir
EOF
fi

token="$(trim_text "$(probe_value token "$probe_file")")"
browser_url="$current_url"
if [[ -n "$token" ]]; then
    browser_url="${current_url}?token=${token}"
fi

printf 'web_pid=%s\n' "$current_pid"
printf 'web_url=%s\n' "$current_url"
printf 'web_browser_url=%s\n' "$browser_url"
printf 'web_log_file=%s\n' "$log_file"

if [[ "$open_browser" == "1" ]]; then
    require_cmd open
    open "$browser_url" >/dev/null 2>&1 || {
        echo "failed to open dev url: $browser_url" >&2
        exit 1
    }
fi

echo "[ok] webui dev server ready"
