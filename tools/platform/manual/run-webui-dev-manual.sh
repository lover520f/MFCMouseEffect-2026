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

trim_ps_value() {
    printf '%s' "${1:-}" | awk '{$1=$1; print}'
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
require_cmd shasum
require_cmd ps
require_cmd pgrep

state_file="/tmp/mfx-webui-dev.state"
log_file="/tmp/mfx-webui-dev.log"
pnpm_bin="$(command -v pnpm)"

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
    local runtime_payload=""
    runtime_payload="$(curl -fsS --max-time 2 "${url}__mfx/dev-runtime" 2>/dev/null || true)"
    [[ "$runtime_payload" == *'"available":true'* ]]
}

list_workspace_vite_pids() {
    pgrep -f "$workspace_dir/node_modules/.*/vite/bin/vite\\.js" 2>/dev/null || true
}

remove_webui_launchctl_jobs() {
    if ! command -v launchctl >/dev/null 2>&1; then
        return 0
    fi
    launchctl list 2>/dev/null \
        | awk '$3 ~ /^com[.]mfcmouseeffect[.]webui-dev[.]/ { print $3 }' \
        | while IFS= read -r label; do
            [[ -n "$label" ]] || continue
            launchctl remove "$label" >/dev/null 2>&1 || true
        done
}

latest_workspace_vite_pid() {
    local latest_pid=""
    local pid=""
    while IFS= read -r pid; do
        pid="$(trim_ps_value "$pid")"
        [[ -n "$pid" ]] || continue
        latest_pid="$pid"
    done < <(list_workspace_vite_pids)
    printf '%s' "$latest_pid"
}

stop_workspace_vite_servers() {
    remove_webui_launchctl_jobs

    local -a vite_pids=()
    local -a all_pids=()
    local pid=""
    while IFS= read -r pid; do
        pid="$(trim_ps_value "$pid")"
        [[ -n "$pid" ]] || continue
        vite_pids+=("$pid")
        all_pids+=("$pid")
        local ppid=""
        ppid="$(trim_ps_value "$(ps -p "$pid" -o ppid= 2>/dev/null || true)")"
        if [[ -n "$ppid" && "$ppid" != "1" ]]; then
            all_pids+=("$ppid")
        fi
    done < <(list_workspace_vite_pids)

    if [[ "${#all_pids[@]}" -eq 0 ]]; then
        return 0
    fi

    local -A seen=()
    local -a unique_pids=()
    for pid in "${all_pids[@]}"; do
        if [[ -n "${seen[$pid]:-}" ]]; then
            continue
        fi
        seen["$pid"]=1
        unique_pids+=("$pid")
    done

    for pid in "${unique_pids[@]}"; do
        kill -TERM "$pid" 2>/dev/null || true
    done
    for _attempt in $(seq 1 40); do
        local alive=0
        for pid in "${unique_pids[@]}"; do
            if kill -0 "$pid" 2>/dev/null; then
                alive=1
                break
            fi
        done
        if [[ "$alive" == "0" ]]; then
            return 0
        fi
        sleep 0.1
    done
    for pid in "${unique_pids[@]}"; do
        kill -KILL "$pid" 2>/dev/null || true
    done
}

current_pid=""
current_url=""
current_config_hash=""
workspace_config_file="$workspace_dir/vite.config.js"
expected_config_hash=""
if [[ -f "$workspace_config_file" ]]; then
    expected_config_hash="$(shasum -a 256 "$workspace_config_file" | awk '{print $1}')"
fi
if [[ -f "$state_file" ]]; then
    current_pid="$(trim_text "$(probe_value pid "$state_file")")"
    current_url="$(trim_text "$(probe_value url "$state_file")")"
    current_config_hash="$(trim_text "$(probe_value config_hash "$state_file")")"
fi

reuse_existing=0
if [[ -n "$current_pid" ]] \
    && kill -0 "$current_pid" 2>/dev/null \
    && url_alive "$current_url" \
    && [[ -n "$expected_config_hash" ]] \
    && [[ "$current_config_hash" == "$expected_config_hash" ]]; then
    reuse_existing=1
else
    rm -f "$state_file"
fi

if [[ "$reuse_existing" != "1" ]]; then
    stop_workspace_vite_servers
    : >"$log_file"
    if command -v launchctl >/dev/null 2>&1; then
        launch_label="com.mfcmouseeffect.webui-dev.$$"
        launchctl remove "$launch_label" >/dev/null 2>&1 || true
        launchctl submit \
            -l "$launch_label" \
            -o "$log_file" \
            -e "$log_file" \
            -- /usr/bin/env \
                "PATH=$PATH" \
                "MFX_WEBUI_DEV_LAUNCH_PID_FILE=$state_file.pid" \
                "MFX_WEBUI_DEV_WORKSPACE_DIR=$workspace_dir" \
                "MFX_WEBUI_DEV_PNPM_BIN=$pnpm_bin" \
                "MFX_WEBUI_DEV_HOST=$preferred_host" \
                /bin/sh -c 'echo "$$" > "$MFX_WEBUI_DEV_LAUNCH_PID_FILE"; cd "$MFX_WEBUI_DEV_WORKSPACE_DIR"; exec "$MFX_WEBUI_DEV_PNPM_BIN" run dev -- --host "$MFX_WEBUI_DEV_HOST"'
    elif command -v setsid >/dev/null 2>&1; then
        (
            cd "$workspace_dir"
            nohup setsid pnpm run dev -- --host "$preferred_host" >"$log_file" 2>&1 < /dev/null &
            echo "$!" > "$state_file.pid"
        )
    else
        (
            cd "$workspace_dir"
            nohup pnpm run dev -- --host "$preferred_host" >"$log_file" 2>&1 < /dev/null &
            echo "$!" > "$state_file.pid"
        )
    fi
    current_pid="$(trim_text "$(cat "$state_file.pid" 2>/dev/null || true)")"
    rm -f "$state_file.pid"

    current_url=""
    vite_pid=""
    dev_runtime_ready=0
    for _attempt in $(seq 1 120); do
        current_url="$(trim_text "$(extract_url_from_log "$log_file")")"
        vite_pid="$(latest_workspace_vite_pid)"
        if [[ -n "$current_url" ]] && [[ -n "$vite_pid" ]] && url_alive "$current_url"; then
            current_pid="$vite_pid"
            dev_runtime_ready=1
            break
        fi
        sleep 0.25
    done

    if [[ -z "$current_url" || "$dev_runtime_ready" != "1" ]]; then
        echo "failed to discover Vite dev URL; see $log_file" >&2
        if [[ -n "$current_url" ]]; then
            echo "discovered URL was not runtime-ready: $current_url" >&2
        fi
        exit 1
    fi

    cat >"$state_file" <<EOF
pid=$current_pid
url=$current_url
config_hash=$expected_config_hash
launch_label=${launch_label:-}
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
