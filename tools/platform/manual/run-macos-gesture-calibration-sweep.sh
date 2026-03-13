#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd -- "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd -- "$script_dir/../../.." && pwd)"

source "$repo_root/tools/platform/regression/lib/common.sh"
source "$repo_root/tools/platform/regression/lib/build.sh"
source "$repo_root/tools/platform/manual/lib/macos_core_host.sh"

usage() {
    cat <<'EOF'
Usage:
  tools/platform/manual/run-macos-gesture-calibration-sweep.sh [options]

Options:
  --build-dir <path>          Build directory (default: /tmp/mfx-platform-macos-core-build)
  --log-file <path>           Host log path (default: /tmp/mfx-core-gesture-calibration.log)
  --probe-file <path>         Probe file path (default: /tmp/mfx-core-gesture-calibration.probe)
  --output-md <path>          Output markdown (default: docs/automation/gesture-calibration-baseline.md)
  --jobs <num>                Build jobs (sets MFX_BUILD_JOBS)
  --skip-build                Skip cmake configure/build
  -h, --help                  Show this help
EOF
}

build_dir="/tmp/mfx-platform-macos-core-build"
log_file="/tmp/mfx-core-gesture-calibration.log"
probe_file="/tmp/mfx-core-gesture-calibration.probe"
output_md="$repo_root/docs/automation/gesture-calibration-baseline.md"
skip_build=0
build_jobs=""

while [[ $# -gt 0 ]]; do
    case "$1" in
    --build-dir)
        [[ $# -ge 2 ]] || mfx_fail "missing value for --build-dir"
        build_dir="$2"
        shift 2
        ;;
    --log-file)
        [[ $# -ge 2 ]] || mfx_fail "missing value for --log-file"
        log_file="$2"
        shift 2
        ;;
    --probe-file)
        [[ $# -ge 2 ]] || mfx_fail "missing value for --probe-file"
        probe_file="$2"
        shift 2
        ;;
    --output-md)
        [[ $# -ge 2 ]] || mfx_fail "missing value for --output-md"
        output_md="$2"
        shift 2
        ;;
    --jobs)
        [[ $# -ge 2 ]] || mfx_fail "missing value for --jobs"
        build_jobs="$2"
        shift 2
        ;;
    --skip-build)
        skip_build=1
        shift
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

if [[ "$OSTYPE" != darwin* ]]; then
    mfx_fail "this script is macOS-only"
fi

mfx_manual_apply_build_jobs_env "$build_jobs" "--jobs"
mfx_require_cmd curl
mfx_require_cmd node

tmp_dir="$(mktemp -d)"
cleanup() {
    rm -rf "$tmp_dir" >/dev/null 2>&1 || true
    mfx_release_lock
    mfx_manual_stop_core_host "$MFX_MANUAL_HOST_PID"
}
trap cleanup EXIT

mfx_manual_acquire_entry_host_lock
mfx_manual_prepare_core_host_binary "$repo_root" "$build_dir" "$skip_build"
host_bin="$MFX_MANUAL_HOST_BIN"

declare -a host_env
host_env+=("MFX_ENABLE_AUTOMATION_SCOPE_TEST_API=1")

start_status=0
mfx_manual_start_core_host "$host_bin" "$probe_file" "$log_file" "${host_env[@]}" || start_status=$?
if [[ "$start_status" -eq 2 ]]; then
    mfx_ok "gesture calibration skipped: $MFX_MANUAL_STARTUP_SKIP_REASON"
    exit 0
fi
if [[ "$start_status" -ne 0 ]]; then
    exit "$start_status"
fi

token_header=("x-mfcmouseeffect-token: $MFX_MANUAL_SETTINGS_TOKEN")
endpoint="$MFX_MANUAL_BASE_URL/api/automation/test-gesture-similarity"

post_case() {
    local payload="$1"
    local out_file="$2"
    local code
    code="$(mfx_http_code "$out_file" "$endpoint" \
        -X POST \
        -H "${token_header[0]}" \
        -H "Content-Type: application/json" \
        -d "$payload")"
    [[ "$code" == "200" ]] || return 2
    return 0
}

expect_case() {
    local out_file="$1"
    local mode="$2"
    local expected="$3"
    node -e '
const fs = require("fs");
const [file, mode, expected] = process.argv.slice(1);
const data = JSON.parse(fs.readFileSync(file, "utf8"));
if (mode === "accepted_best") {
  process.exit((data.accepted === true && String(data.best_id || "") === expected) ? 0 : 1);
}
if (mode === "rejected_reason") {
  process.exit((data.accepted === false && String(data.reason || "") === expected) ? 0 : 1);
}
process.exit(2);
' "$out_file" "$mode" "$expected"
}

make_windowed_right_payload() {
    local margin="$1"
    cat <<EOF
{"threshold":72,"margin":$margin,"options":{"enable_window_search":true},"captured_strokes":[[{"x":8,"y":72},{"x":14,"y":68},{"x":20,"y":64},{"x":26,"y":58},{"x":30,"y":52},{"x":34,"y":50},{"x":44,"y":50},{"x":54,"y":50},{"x":64,"y":50},{"x":74,"y":50},{"x":84,"y":50},{"x":90,"y":49},{"x":94,"y":48}]],"candidates":[{"id":"right","source":"preset","action_id":"right"},{"id":"left","source":"preset","action_id":"left"},{"id":"v","source":"preset","action_id":"diag_down_right_diag_up_right"}]}
EOF
}

make_v_payload() {
    local margin="$1"
    cat <<EOF
{"threshold":72,"margin":$margin,"options":{"enable_window_search":true},"captured_strokes":[[{"x":16,"y":20},{"x":30,"y":48},{"x":50,"y":82},{"x":66,"y":46},{"x":84,"y":20}]],"candidates":[{"id":"v","source":"preset","action_id":"diag_down_right_diag_up_right"},{"id":"w","source":"preset","action_id":"diag_down_right_diag_up_right_diag_down_right"},{"id":"down_right","source":"preset","action_id":"down_right"}]}
EOF
}

make_w_payload() {
    local margin="$1"
    cat <<EOF
{"threshold":72,"margin":$margin,"options":{"enable_window_search":true},"captured_strokes":[[{"x":8,"y":18},{"x":26,"y":82},{"x":44,"y":36},{"x":62,"y":82},{"x":84,"y":20}]],"candidates":[{"id":"w","source":"preset","action_id":"diag_down_right_diag_up_right_diag_down_right"},{"id":"v","source":"preset","action_id":"diag_down_right_diag_up_right"},{"id":"right","source":"preset","action_id":"right"}]}
EOF
}

make_ambiguous_payload() {
    local margin="$1"
    cat <<EOF
{"threshold":70,"margin":$margin,"options":{"enable_window_search":true},"captured_strokes":[[{"x":10,"y":60},{"x":30,"y":60},{"x":50,"y":60},{"x":70,"y":60},{"x":90,"y":60}]],"candidates":[{"id":"same_a","source":"custom","template_strokes":[[{"x":10,"y":60},{"x":30,"y":60},{"x":50,"y":60},{"x":70,"y":60},{"x":90,"y":60}]]},{"id":"same_b","source":"custom","template_strokes":[[{"x":10,"y":60},{"x":30,"y":60},{"x":50,"y":60},{"x":70,"y":60},{"x":90,"y":60}]]}]}
EOF
}

make_custom_order_payload() {
    local margin="$1"
    local min_len="$2"
    cat <<EOF
{"threshold":72,"margin":$margin,"options":{"enable_window_search":true,"strict_stroke_count":true,"strict_stroke_order":true,"min_effective_stroke_length_px":$min_len},"captured_strokes":[[{"x":14,"y":18},{"x":88,"y":18}],[{"x":88,"y":78},{"x":18,"y":78}]],"candidates":[{"id":"order_ok","source":"custom","template_strokes":[[{"x":12,"y":20},{"x":90,"y":20}],[{"x":88,"y":80},{"x":16,"y":80}]]},{"id":"order_bad","source":"custom","template_strokes":[[{"x":88,"y":80},{"x":16,"y":80}],[{"x":12,"y":20},{"x":90,"y":20}]]}]}
EOF
}

make_noise_payload() {
    local margin="$1"
    local min_len="$2"
    cat <<EOF
{"threshold":70,"margin":$margin,"options":{"enable_window_search":true,"strict_stroke_count":true,"strict_stroke_order":true,"min_effective_stroke_length_px":$min_len},"captured_strokes":[[{"x":10,"y":10},{"x":18,"y":10}]],"candidates":[{"id":"short_noise","source":"custom","template_strokes":[[{"x":10,"y":10},{"x":90,"y":10}]]}]}
EOF
}

margins=(2 3 4 5 6)
min_lengths=(12 18 24 30)

results_csv="$tmp_dir/results.csv"
printf 'margin,min_len,pass,total\n' >"$results_csv"
case_windowed_pass=0
case_v_pass=0
case_w_pass=0
case_ambiguous_pass=0
case_order_pass=0
case_noise_pass=0
combo_count=0
w_probe_json=""

for margin in "${margins[@]}"; do
    for min_len in "${min_lengths[@]}"; do
        ((combo_count+=1))
        pass_count=0
        total_count=6

        payload="$(make_windowed_right_payload "$margin")"
        out="$tmp_dir/case-windowed-right-$margin-$min_len.json"
        post_case "$payload" "$out"
        if expect_case "$out" "accepted_best" "right"; then
            ((pass_count+=1))
            ((case_windowed_pass+=1))
        fi

        payload="$(make_v_payload "$margin")"
        out="$tmp_dir/case-v-$margin-$min_len.json"
        post_case "$payload" "$out"
        if expect_case "$out" "accepted_best" "v"; then
            ((pass_count+=1))
            ((case_v_pass+=1))
        fi

        payload="$(make_w_payload "$margin")"
        out="$tmp_dir/case-w-$margin-$min_len.json"
        post_case "$payload" "$out"
        if [[ -z "$w_probe_json" && "$margin" -eq 4 && "$min_len" -eq 18 ]]; then
            w_probe_json="$out"
        fi
        if expect_case "$out" "accepted_best" "w"; then
            ((pass_count+=1))
            ((case_w_pass+=1))
        fi

        payload="$(make_ambiguous_payload "$margin")"
        out="$tmp_dir/case-ambiguous-$margin-$min_len.json"
        post_case "$payload" "$out"
        if expect_case "$out" "rejected_reason" "ambiguous"; then
            ((pass_count+=1))
            ((case_ambiguous_pass+=1))
        fi

        payload="$(make_custom_order_payload "$margin" "$min_len")"
        out="$tmp_dir/case-order-$margin-$min_len.json"
        post_case "$payload" "$out"
        if expect_case "$out" "accepted_best" "order_ok"; then
            ((pass_count+=1))
            ((case_order_pass+=1))
        fi

        payload="$(make_noise_payload "$margin" "$min_len")"
        out="$tmp_dir/case-noise-$margin-$min_len.json"
        post_case "$payload" "$out"
        if expect_case "$out" "rejected_reason" "no_valid_candidate"; then
            ((pass_count+=1))
            ((case_noise_pass+=1))
        fi

        printf '%s,%s,%s,%s\n' "$margin" "$min_len" "$pass_count" "$total_count" >>"$results_csv"
    done
done

best_row="$(
    awk -F',' '
    NR==1 {next}
    {
      margin=$1+0;
      min_len=$2+0;
      pass=$3+0;
      dist=((margin-4)>=0?(margin-4):-(margin-4)) + (((min_len-18)>=0?(min_len-18):-(min_len-18))/6.0);
      if (pass > best_pass || (pass==best_pass && dist < best_dist)) {
        best_pass=pass;
        best_dist=dist;
        best_margin=margin;
        best_min=min_len;
      }
    }
    END {
      printf "%d,%d,%d", best_margin, best_min, best_pass;
    }' "$results_csv"
)"

best_margin="$(printf '%s' "$best_row" | cut -d',' -f1)"
best_min_len="$(printf '%s' "$best_row" | cut -d',' -f2)"
best_pass="$(printf '%s' "$best_row" | cut -d',' -f3)"

generated_at="$(date '+%Y-%m-%d %H:%M:%S %z')"
w_probe_line="N/A"
if [[ -n "$w_probe_json" && -f "$w_probe_json" ]]; then
    w_probe_line="$(
        node -e '
const fs = require("fs");
const file = process.argv[1];
const data = JSON.parse(fs.readFileSync(file, "utf8"));
const best = Number(data.best_score ?? -1);
const runner = Number(data.runner_up_score ?? -1);
const delta = (best >= 0 && runner >= 0) ? (best - runner).toFixed(2) : "n/a";
console.log(`reason=${data.reason || "-"} best_id=${data.best_id || "-"} best=${best.toFixed(2)} runner=${runner.toFixed(2)} delta=${delta}`);
' "$w_probe_json"
    )"
fi
mkdir -p "$(dirname "$output_md")"
{
    echo "# Gesture Calibration Baseline"
    echo
    echo "Updated: $generated_at"
    echo
    echo "## Sweep Grid"
    echo
    echo "- margins: ${margins[*]}"
    echo "- custom min effective stroke length(px): ${min_lengths[*]}"
    echo "- dataset cases: 6 (windowed-right, v, w, ambiguous-reject, custom-order, short-noise-reject)"
    echo
    echo "## Results"
    echo
    echo "| Margin | Custom Min Len(px) | Pass | Total |"
    echo "|---:|---:|---:|---:|"
    awk -F',' 'NR>1 {printf "| %d | %d | %d | %d |\n", $1, $2, $3, $4}' "$results_csv"
    echo
    echo "## Recommended Baseline"
    echo
    echo "- ambiguity margin env (\`MFX_GESTURE_AMBIGUITY_MARGIN\`): **$best_margin**"
    echo "- custom min effective stroke env (\`MFX_GESTURE_CUSTOM_MIN_EFFECTIVE_STROKE_PX\`): **$best_min_len**"
    echo "- pass: **$best_pass/6**"
    if [[ "$best_pass" -lt 6 ]]; then
      echo "- note: dataset not fully separable under current sweep grid; keep adaptive margin and continue tuning with larger sample set."
    fi
    echo
    echo "## Case Pass Rate"
    echo
    echo "- windowed-right: $case_windowed_pass/$combo_count"
    echo "- preset-v: $case_v_pass/$combo_count"
    echo "- preset-w: $case_w_pass/$combo_count"
    echo "- ambiguous-reject: $case_ambiguous_pass/$combo_count"
    echo "- custom-order: $case_order_pass/$combo_count"
    echo "- short-noise-reject: $case_noise_pass/$combo_count"
    echo
    echo "## W Probe (margin=4, min_len=18)"
    echo
    echo "- $w_probe_line"
    echo
    echo "## Reproduce"
    echo
    echo '```bash'
    echo "./tools/platform/manual/run-macos-gesture-calibration-sweep.sh --skip-build"
    echo '```'
} >"$output_md"

printf 'calibration_output=%s\n' "$output_md"
printf 'recommended_margin=%s\n' "$best_margin"
printf 'recommended_custom_min_len_px=%s\n' "$best_min_len"
printf 'recommended_pass=%s/6\n' "$best_pass"
mfx_ok "gesture calibration sweep completed"
