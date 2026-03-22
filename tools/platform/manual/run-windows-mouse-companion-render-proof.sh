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
  --appearance-profile-path <p> Optional pet-appearance.json path for appearance expectation checks
  --route <proof|sweep>         Route kind (default: sweep)
  --preset <name>               Named preset (currently: real-preview-smoke, combo-persona-acceptance)
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
  --expect-appearance-profile-match <bool> Require renderer diagnostics to match pet-appearance.json
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

print_combo_persona_acceptance_hint() {
    cat <<'EOF'
[mfx:info] combo-persona-acceptance preset
  - stage 1:
    - runs the same backend/preview/frame smoke gate as real-preview-smoke
  - stage 2:
    - switches runtime appearance_profile_path across the synced combo-only JSON files
    - validates runtime appearance diagnostics for each combo case
  - sync-safe:
    - does not edit the receive-only synced repo on Windows
EOF
}

resolve_default_appearance_profile_path() {
    local candidates=(
        "$repo_root/MFCMouseEffect/Assets/Pet3D/source/pet-appearance.json"
        "$repo_root/Assets/Pet3D/source/pet-appearance.json"
    )
    local candidate
    for candidate in "${candidates[@]}"; do
        if [[ -f "$candidate" ]]; then
            printf '%s\n' "$candidate"
            return 0
        fi
    done
    printf '%s\n' "${candidates[0]}"
}

resolve_combo_persona_profile_cases() {
    cat <<EOF
cream-moon:$repo_root/MFCMouseEffect/Assets/Pet3D/source/pet-appearance.combo-cream-moon.json
night-leaf:$repo_root/MFCMouseEffect/Assets/Pet3D/source/pet-appearance.combo-night-leaf.json
strawberry-ribbon-bow:$repo_root/MFCMouseEffect/Assets/Pet3D/source/pet-appearance.combo-strawberry-ribbon-bow.json
EOF
}

read_current_appearance_profile_path() {
    local response_file="$1"
    local http_code
    http_code="$(mfx_http_code "$response_file" "$base_url/api/state" \
        -H "x-mfcmouseeffect-token: $token")"
    mfx_assert_eq "$http_code" "200" "windows mouse companion current state route"
    python3 - "$response_file" <<'PY'
import json
import sys
with open(sys.argv[1], "r", encoding="utf-8") as fp:
    data = json.load(fp)
print(((data.get("mouse_companion") or {}).get("appearance_profile_path") or "").strip())
PY
}

apply_appearance_profile_path() {
    local target_path="$1"
    local body
    body="$(python3 - "$target_path" <<'PY'
import json
import sys
print(json.dumps({
    "mouse_companion": {
        "appearance_profile_path": sys.argv[1]
    }
}, ensure_ascii=False))
PY
)"
    local response_file
    response_file="$(mktemp)"
    local http_code
    http_code="$(mfx_http_code "$response_file" "$base_url/api/state" \
        -X POST \
        -H "x-mfcmouseeffect-token: $token" \
        -H "Content-Type: application/json" \
        --data "$body")"
    rm -f "$response_file"
    mfx_assert_eq "$http_code" "200" "windows mouse companion apply appearance profile path"
}

base_url=""
token=""
appearance_profile_path=""
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
expect_appearance_profile_match="false"
json_output=""
run_combo_persona_matrix="false"

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
    --appearance-profile-path)
        mfx_require_option_value "$1" "${2:-}"
        appearance_profile_path="$2"
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
    --expect-appearance-profile-match)
        mfx_require_option_value "$1" "${2:-}"
        expect_appearance_profile_match="$2"
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
    combo-persona-acceptance)
        route_kind="sweep"
        wait_for_frame_ms=120
        expect_frame_advance="true"
        expected_backend="real"
        expect_preview_active="true"
        expect_appearance_profile_match="true"
        run_combo_persona_matrix="true"
        ;;
    *)
        mfx_fail "invalid --preset value: $preset_name (expected: real-preview-smoke or combo-persona-acceptance)"
        ;;
esac

if [[ "$preset_name" == "real-preview-smoke" ]]; then
    print_real_preview_smoke_hint
elif [[ "$preset_name" == "combo-persona-acceptance" ]]; then
    print_real_preview_smoke_hint
    print_combo_persona_acceptance_hint
fi

if [[ "$run_combo_persona_matrix" != "true" && "$expect_appearance_profile_match" == "true" && -z "$appearance_profile_path" ]]; then
    appearance_profile_path="$(resolve_default_appearance_profile_path)"
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

if [[ "$run_combo_persona_matrix" == "true" ]]; then
    state_response_file="$(mktemp)"
    combo_failed_labels=()
    original_appearance_profile_path="$(read_current_appearance_profile_path "$state_response_file")"
    trap 'rm -f "$payload_file" "$response_file" "$state_response_file"' EXIT
    while IFS=: read -r case_label case_path; do
        [[ -n "$case_label" ]] || continue
        if [[ ! -f "$case_path" ]]; then
            printf '[mfx:fail] combo persona case %s: missing profile %s\n' "$case_label" "$case_path"
            combo_failed_labels+=("$case_label")
            continue
        fi
        printf '[mfx:info] combo persona case %s: applying %s\n' "$case_label" "$case_path"
        apply_appearance_profile_path "$case_path"
        child_args=(
            --base-url "$base_url"
            --token "$token"
            --appearance-profile-path "$case_path"
            --route "$route_kind"
            --event "$event_name"
            --x "$x"
            --y "$y"
            --button "$button"
            --delta "$delta"
            --hold-ms "$hold_ms"
            --wait-for-frame-ms "$wait_for_frame_ms"
            --expect-frame-advance "$expect_frame_advance"
            --expected-backend "$expected_backend"
            --expect-preview-active "$expect_preview_active"
            --expect-appearance-profile-match true
        )
        if [[ -n "$json_output" ]]; then
            child_args+=(--json-output "${json_output}.${case_label}.json")
        fi
        if ! bash "$0" "${child_args[@]}"; then
            combo_failed_labels+=("$case_label")
        fi
    done < <(resolve_combo_persona_profile_cases)
    if [[ -n "$original_appearance_profile_path" ]]; then
        printf '[mfx:info] restoring appearance profile path: %s\n' "$original_appearance_profile_path"
        apply_appearance_profile_path "$original_appearance_profile_path"
    fi
    if (( ${#combo_failed_labels[@]} > 0 )); then
        mfx_fail "combo persona matrix failed: ${combo_failed_labels[*]}"
    fi
    mfx_ok "combo persona matrix"
    exit 0
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

python3 - "$response_file" "$route_kind" "$expect_appearance_profile_match" "$appearance_profile_path" <<'PY'
import json
import sys

response_file, route_kind, expect_appearance_profile_match, appearance_profile_path = sys.argv[1:]
with open(response_file, "r", encoding="utf-8") as fp:
    data = json.load(fp)

def parse_bool(value: str) -> bool:
    return value.strip().lower() in {"1", "true", "yes", "on"}

def resolve_accessory_family(accessory_ids):
    values = [str(item) for item in (accessory_ids or [])]
    for item in values:
        if "moon" in item:
            return "moon"
    for item in values:
        if "leaf" in item:
            return "leaf"
    for item in values:
        if "ribbon" in item or "bow" in item:
            return "ribbon_bow"
    return "star" if values else "none"

def resolve_combo_preset(skin_variant_id: str, accessory_family: str) -> str:
    if skin_variant_id == "cream" and accessory_family == "moon":
        return "dreamy"
    if skin_variant_id == "night" and accessory_family == "leaf":
        return "agile"
    if skin_variant_id == "strawberry" and accessory_family == "ribbon_bow":
        return "charming"
    return "none"

expected_appearance = None
if parse_bool(expect_appearance_profile_match):
    with open(appearance_profile_path, "r", encoding="utf-8") as fp:
        root = json.load(fp)
    requested_preset_id = str(root.get("activePreset", "") or "")
    resolved_node = None
    resolved_preset_id = ""
    if requested_preset_id and isinstance(root.get("presets"), list):
        for preset in root["presets"]:
            if isinstance(preset, dict) and str(preset.get("id", "") or "") == requested_preset_id:
                resolved_node = preset
                resolved_preset_id = requested_preset_id
                break
    if resolved_node is None and isinstance(root.get("default"), dict):
        resolved_node = root["default"]
        resolved_preset_id = "default"
    if resolved_node is None:
        raise SystemExit(f"[mfx:fail] appearance profile did not resolve to active preset or default: {appearance_profile_path}")
    accessory_ids = [str(item) for item in (resolved_node.get("enabledAccessoryIds") or [])]
    accessory_family = resolve_accessory_family(accessory_ids)
    expected_appearance = {
        "appearance_requested_preset_id": requested_preset_id,
        "appearance_resolved_preset_id": resolved_preset_id,
        "appearance_skin_variant_id": str(resolved_node.get("skinVariantId", "") or "default"),
        "appearance_accessory_ids": accessory_ids,
        "appearance_accessory_family": accessory_family,
        "appearance_combo_preset": resolve_combo_preset(
            str(resolved_node.get("skinVariantId", "") or "default"),
            accessory_family,
        ),
    }
    print(
        "[mfx:info] appearance expectation"
        f" requested={expected_appearance['appearance_requested_preset_id']}"
        f" resolved={expected_appearance['appearance_resolved_preset_id']}"
        f" skin={expected_appearance['appearance_skin_variant_id']}"
        f" accessory_family={expected_appearance['appearance_accessory_family']}"
        f" combo={expected_appearance['appearance_combo_preset']}"
    )

def compare_appearance(node, label):
    if expected_appearance is None:
        return []
    if not isinstance(node, dict):
        return [f"{label}:missing_node"]
    mismatches = []
    for key in (
        "appearance_requested_preset_id",
        "appearance_resolved_preset_id",
        "appearance_skin_variant_id",
        "appearance_accessory_family",
        "appearance_combo_preset",
    ):
        actual = str(node.get(key, "") or "")
        expected = str(expected_appearance[key])
        if actual != expected:
            mismatches.append(f"{label}:{key} expected={expected} actual={actual}")
    actual_accessory_ids = [str(item) for item in (node.get("appearance_accessory_ids") or [])]
    if actual_accessory_ids != expected_appearance["appearance_accessory_ids"]:
        mismatches.append(
            f"{label}:appearance_accessory_ids expected=[{','.join(expected_appearance['appearance_accessory_ids'])}]"
            f" actual=[{','.join(actual_accessory_ids)}]"
        )
    return mismatches

if route_kind == "sweep":
    summary = data.get("summary", {})
    all_expectations_met = bool(summary.get("all_expectations_met", False))
    appearance_failures = []
    if expected_appearance is not None:
        for item in data.get("results", []):
            appearance_failures.extend(compare_appearance(((item.get("proof") or {}).get("renderer_runtime_after") or {}), f"{item.get('event','')}:renderer_runtime_after"))
            appearance_failures.extend(compare_appearance((item.get("real_renderer_preview") or {}), f"{item.get('event','')}:real_renderer_preview"))
    appearance_expectation_met = len(appearance_failures) == 0
    all_expectations_met = all_expectations_met and appearance_expectation_met
    label = "[mfx:ok] render proof sweep" if all_expectations_met else "[mfx:fail] render proof sweep"
    print(label)
    print(
        f"  - expectations={summary.get('expectation_met_count','')}/{summary.get('expectation_requested_count','')}"
        f" frame_advanced={summary.get('frame_advanced_count','')}"
        f" backend_checks={summary.get('backend_expectation_met_count','')}/{summary.get('backend_expectation_count','')}"
        f" preview_checks={summary.get('preview_expectation_met_count','')}/{summary.get('preview_expectation_count','')}"
        f" appearance_check={appearance_expectation_met}"
    )
    for item in data.get("results", []):
        event = item.get("event", "")
        proof = item.get("proof", {})
        delta = proof.get("renderer_runtime_delta", {})
        preview = item.get("real_renderer_preview", {}) or {}
        print(
            f"  - {event}: status={proof.get('renderer_runtime_expectation_status','')}"
            f" frame_delta={delta.get('frame_count_delta','')}"
            f" backend={item.get('selected_renderer_backend','')}"
            f" preview_active={preview.get('preview_active','')}"
            f" preset={preview.get('appearance_requested_preset_id','')}->{preview.get('appearance_resolved_preset_id','')}"
            f" combo={preview.get('appearance_combo_preset','')}"
        )
    for failure in appearance_failures:
        print(f"  - appearance_mismatch: {failure}")
    if not all_expectations_met:
        raise SystemExit(1)
else:
    delta = data.get("renderer_runtime_delta", {})
    frame_expectation_met = bool(data.get("renderer_runtime_expectation_met", True))
    backend_expectation_met = bool(data.get("backend_expectation_met", True))
    preview_expectation_met = bool(data.get("preview_expectation_met", True))
    all_expectations_met = bool(data.get("all_expectations_met", True))
    appearance_failures = []
    if expected_appearance is not None:
        appearance_failures.extend(compare_appearance((data.get("renderer_runtime_after") or {}), "renderer_runtime_after"))
        appearance_failures.extend(compare_appearance((data.get("real_renderer_preview") or {}), "real_renderer_preview"))
    appearance_expectation_met = len(appearance_failures) == 0
    all_expectations_met = all_expectations_met and appearance_expectation_met
    label = "[mfx:ok] render proof" if all_expectations_met else "[mfx:fail] render proof"
    print(label)
    preview = data.get("real_renderer_preview", {}) or {}
    print(
        f"  - status={data.get('renderer_runtime_expectation_status','')}"
        f" frame_delta={delta.get('frame_count_delta','')}"
        f" backend={data.get('selected_renderer_backend','')}"
        f" preview_active={preview.get('preview_active','')}"
        f" frame_check={frame_expectation_met}"
        f" backend_check={backend_expectation_met}"
        f" preview_check={preview_expectation_met}"
        f" appearance_check={appearance_expectation_met}"
    )
    print(
        f"  - appearance preset={preview.get('appearance_requested_preset_id','')}->{preview.get('appearance_resolved_preset_id','')}"
        f" skin={preview.get('appearance_skin_variant_id','')}"
        f" accessory_family={preview.get('appearance_accessory_family','')}"
        f" combo={preview.get('appearance_combo_preset','')}"
    )
    for failure in appearance_failures:
        print(f"  - appearance_mismatch: {failure}")
    if not all_expectations_met:
        raise SystemExit(1)
PY
