#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd -- "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd -- "$script_dir/../../.." && pwd)"

source "$repo_root/tools/platform/regression/lib/common.sh"

usage() {
    cat <<'EOF'
Usage:
  tools/platform/manual/run-windows-mouse-companion-render-proof.sh [options]

Options:
  --base-url <url>              Required API base URL, e.g. http://127.0.0.1:8787
  --token <token>               Required x-mfcmouseeffect-token value
  --route <proof|sweep>         Route kind (default: sweep)
  --preset <name>               Named preset (currently: real-preview-smoke)
  --event <name>                Single proof event when --route proof (default: status)
  --x <int>                     Pointer x (default: 640)
  --y <int>                     Pointer y (default: 360)
  --button <int>                Button id (default: 1)
  --delta <int>                 Scroll delta (default: 120)
  --hold-ms <int>               Hold duration (default: 420)
  --wait-for-frame-ms <int>     Proof wait budget (default: 120)
  --expect-frame-advance <bool> Expect frame advance (default: true)
  --expected-backend <name>     Require selected backend to match this name
  --expect-preview-active <bool> Require real preview active during proof (default: false)
  --json-output <path>          Save full json response to file
  -h, --help                    Show this help
EOF
}

print_real_preview_smoke_hint() {
    cat <<'EOF'
[mfx:info] real-preview-smoke preset
  - expected env:
    - MFX_ENABLE_MOUSE_COMPANION_TEST_API=1
    - MFX_WIN32_MOUSE_COMPANION_REAL_RENDERER_ENABLE=1
    - optional: MFX_WIN32_MOUSE_COMPANION_RENDERER_BACKEND=real
  - expected gate:
    - selected backend should resolve to real
    - preview should be active
    - sweep should advance frames on action rows
EOF
}

base_url=""
token=""
route_kind="sweep"
preset_name=""
event_name="status"
x=640
y=360
button=1
delta=120
hold_ms=420
wait_for_frame_ms=120
expect_frame_advance="true"
expected_backend=""
expect_preview_active="false"
json_output=""

while [[ $# -gt 0 ]]; do
    case "$1" in
    --base-url)
        mfx_require_option_value "$1" "${2:-}"
        base_url="$2"
        shift 2
        ;;
    --token)
        mfx_require_option_value "$1" "${2:-}"
        token="$2"
        shift 2
        ;;
    --route)
        mfx_require_option_value "$1" "${2:-}"
        route_kind="$2"
        shift 2
        ;;
    --preset)
        mfx_require_option_value "$1" "${2:-}"
        preset_name="$2"
        shift 2
        ;;
    --event)
        mfx_require_option_value "$1" "${2:-}"
        event_name="$2"
        shift 2
        ;;
    --x)
        mfx_require_option_value "$1" "${2:-}"
        x="$2"
        shift 2
        ;;
    --y)
        mfx_require_option_value "$1" "${2:-}"
        y="$2"
        shift 2
        ;;
    --button)
        mfx_require_option_value "$1" "${2:-}"
        button="$2"
        shift 2
        ;;
    --delta)
        mfx_require_option_value "$1" "${2:-}"
        delta="$2"
        shift 2
        ;;
    --hold-ms)
        mfx_require_option_value "$1" "${2:-}"
        hold_ms="$2"
        shift 2
        ;;
    --wait-for-frame-ms)
        mfx_require_option_value "$1" "${2:-}"
        wait_for_frame_ms="$2"
        shift 2
        ;;
    --expect-frame-advance)
        mfx_require_option_value "$1" "${2:-}"
        expect_frame_advance="$2"
        shift 2
        ;;
    --expected-backend)
        mfx_require_option_value "$1" "${2:-}"
        expected_backend="$2"
        shift 2
        ;;
    --expect-preview-active)
        mfx_require_option_value "$1" "${2:-}"
        expect_preview_active="$2"
        shift 2
        ;;
    --json-output)
        mfx_require_option_value "$1" "${2:-}"
        json_output="$2"
        shift 2
        ;;
    -h|--help)
        usage
        exit 0
        ;;
    *)
        mfx_fail "unknown argument: $1"
        ;;
    esac
done

mfx_require_cmd curl
mfx_require_cmd python3

[[ -n "$base_url" ]] || mfx_fail "missing required --base-url"
[[ -n "$token" ]] || mfx_fail "missing required --token"

case "$route_kind" in
    proof|sweep) ;;
    *)
        mfx_fail "invalid --route value: $route_kind (expected: proof|sweep)"
        ;;
esac

case "$preset_name" in
    "")
        ;;
    real-preview-smoke)
        route_kind="sweep"
        wait_for_frame_ms=120
        expect_frame_advance="true"
        expected_backend="real"
        expect_preview_active="true"
        ;;
    *)
        mfx_fail "invalid --preset value: $preset_name (expected: real-preview-smoke)"
        ;;
esac

if [[ "$preset_name" == "real-preview-smoke" ]]; then
    print_real_preview_smoke_hint
fi

mfx_require_non_negative_integer() {
    local raw_value="$1"
    local context_name="$2"
    if ! [[ "$raw_value" =~ ^-?[0-9]+$ ]]; then
        mfx_fail "invalid $context_name value: $raw_value"
    fi
}

mfx_require_non_negative_integer "$x" "x"
mfx_require_non_negative_integer "$y" "y"
mfx_require_non_negative_integer "$button" "button"
mfx_require_non_negative_integer "$delta" "delta"
mfx_require_non_negative_integer "$hold_ms" "hold-ms"
mfx_require_non_negative_integer "$wait_for_frame_ms" "wait-for-frame-ms"

endpoint="$base_url/api/mouse-companion/test-render-proof"
if [[ "$route_kind" == "sweep" ]]; then
    endpoint="$base_url/api/mouse-companion/test-render-proof-sweep"
fi

payload_file="$(mktemp)"
response_file="$(mktemp)"
trap 'rm -f "$payload_file" "$response_file"' EXIT

python3 - "$payload_file" "$route_kind" "$event_name" "$x" "$y" "$button" "$delta" "$hold_ms" "$wait_for_frame_ms" "$expect_frame_advance" "$expected_backend" "$expect_preview_active" <<'PY'
import json
import sys

out_file, route_kind, event_name, x, y, button, delta, hold_ms, wait_ms, expect_adv, expected_backend, expect_preview_active = sys.argv[1:]

def parse_bool(value: str) -> bool:
    return value.strip().lower() in {"1", "true", "yes", "on"}

payload = {
    "x": int(x),
    "y": int(y),
    "button": int(button),
    "delta": int(delta),
    "hold_ms": int(hold_ms),
    "wait_for_frame_ms": int(wait_ms),
    "expect_frame_advance": parse_bool(expect_adv),
    "expected_backend": expected_backend,
    "expect_preview_active": parse_bool(expect_preview_active),
}
if route_kind == "proof":
    payload["event"] = event_name

with open(out_file, "w", encoding="utf-8") as fp:
    json.dump(payload, fp, ensure_ascii=False)
PY

http_code="$(mfx_http_code "$response_file" "$endpoint" \
    -X POST \
    -H "x-mfcmouseeffect-token: $token" \
    -H "Content-Type: application/json" \
    --data @"$payload_file")"

mfx_assert_eq "$http_code" "200" "windows mouse companion render proof route"

if [[ -n "$json_output" ]]; then
    cp "$response_file" "$json_output"
    mfx_ok "saved proof json: $json_output"
fi

python3 - "$response_file" "$route_kind" <<'PY'
import json
import sys

response_file, route_kind = sys.argv[1:]
with open(response_file, "r", encoding="utf-8") as fp:
    data = json.load(fp)

if route_kind == "sweep":
    summary = data.get("summary", {})
    all_expectations_met = bool(summary.get("all_expectations_met", False))
    label = "[mfx:ok] render proof sweep" if all_expectations_met else "[mfx:fail] render proof sweep"
    print(label)
    print(
        f"  - expectations={summary.get('expectation_met_count','')}/{summary.get('expectation_requested_count','')}"
        f" frame_advanced={summary.get('frame_advanced_count','')}"
        f" backend_checks={summary.get('backend_expectation_met_count','')}/{summary.get('backend_expectation_count','')}"
        f" preview_checks={summary.get('preview_expectation_met_count','')}/{summary.get('preview_expectation_count','')}"
    )
    for item in data.get("results", []):
        event = item.get("event", "")
        proof = item.get("proof", {})
        delta = proof.get("renderer_runtime_delta", {})
        print(
            f"  - {event}: status={proof.get('renderer_runtime_expectation_status','')}"
            f" frame_delta={delta.get('frame_count_delta','')}"
            f" backend={item.get('selected_renderer_backend','')}"
            f" preview_active={item.get('real_renderer_preview',{}).get('preview_active','')}"
        )
    if not all_expectations_met:
        raise SystemExit(1)
else:
    delta = data.get("renderer_runtime_delta", {})
    frame_expectation_met = bool(data.get("renderer_runtime_expectation_met", True))
    backend_expectation_met = bool(data.get("backend_expectation_met", True))
    preview_expectation_met = bool(data.get("preview_expectation_met", True))
    all_expectations_met = bool(data.get("all_expectations_met", True))
    label = "[mfx:ok] render proof" if all_expectations_met else "[mfx:fail] render proof"
    print(label)
    print(
        f"  - status={data.get('renderer_runtime_expectation_status','')}"
        f" frame_delta={delta.get('frame_count_delta','')}"
        f" backend={data.get('selected_renderer_backend','')}"
        f" preview_active={data.get('real_renderer_preview',{}).get('preview_active','')}"
        f" frame_check={frame_expectation_met}"
        f" backend_check={backend_expectation_met}"
        f" preview_check={preview_expectation_met}"
    )
    if not all_expectations_met:
        raise SystemExit(1)
PY
