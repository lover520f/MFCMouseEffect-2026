#!/usr/bin/env bash

set -euo pipefail

_mfx_core_http_automation_json_escape() {
    local value="$1"
    printf '%s' "$value" | sed 's/\\/\\\\/g; s/"/\\"/g'
}

_mfx_core_http_automation_parse_first_key_value() {
    local input_file="$1"
    local key_name="$2"
    local output_mode="$3"
    python3 - "$input_file" "$key_name" "$output_mode" <<'PY'
import json
import sys

input_file = sys.argv[1]
key_name = sys.argv[2]
output_mode = sys.argv[3]
missing = object()

try:
    with open(input_file, "r", encoding="utf-8") as f:
        root = json.load(f)
except Exception:
    print("")
    sys.exit(0)

def find_first_key(node, target):
    if isinstance(node, dict):
        for key, value in node.items():
            if key == target:
                return value
            resolved = find_first_key(value, target)
            if resolved is not missing:
                return resolved
    elif isinstance(node, list):
        for item in node:
            resolved = find_first_key(item, target)
            if resolved is not missing:
                return resolved
    return missing

def emit_value(value, mode):
    if mode == "string":
        if isinstance(value, str):
            print(value)
            return
        print("")
        return
    if mode == "uint":
        if isinstance(value, bool):
            print("")
            return
        if isinstance(value, int) and value >= 0:
            print(str(value))
            return
        print("")
        return

    # scalar output mode: align with previous shell parser semantics.
    if isinstance(value, bool):
        print("true" if value else "false")
    elif isinstance(value, (int, float)) and not isinstance(value, bool):
        print(str(value))
    elif value is None:
        print("null")
    elif isinstance(value, str):
        print(json.dumps(value, ensure_ascii=False, separators=(",", ":")))
    else:
        print(json.dumps(value, ensure_ascii=False, separators=(",", ":")))

resolved = find_first_key(root, key_name)
if resolved is missing:
    print("")
    sys.exit(0)

emit_value(resolved, output_mode)
PY
}

_mfx_core_http_automation_parse_section_value() {
    local input_file="$1"
    local section_name="$2"
    local field_name="$3"
    local output_mode="$4"
    python3 - "$input_file" "$section_name" "$field_name" "$output_mode" <<'PY'
import json
import sys

input_file = sys.argv[1]
section_name = sys.argv[2]
field_name = sys.argv[3]
output_mode = sys.argv[4]
missing = object()

try:
    with open(input_file, "r", encoding="utf-8") as f:
        root = json.load(f)
except Exception:
    print("")
    sys.exit(0)

def emit_value(value, mode):
    if mode == "string":
        if isinstance(value, str):
            print(value)
            return
        print("")
        return
    if mode == "uint":
        if isinstance(value, bool):
            print("")
            return
        if isinstance(value, int) and value >= 0:
            print(str(value))
            return
        print("")
        return

    if isinstance(value, bool):
        print("true" if value else "false")
    elif isinstance(value, (int, float)) and not isinstance(value, bool):
        print(str(value))
    elif value is None:
        print("null")
    elif isinstance(value, str):
        print(json.dumps(value, ensure_ascii=False, separators=(",", ":")))
    else:
        print(json.dumps(value, ensure_ascii=False, separators=(",", ":")))

resolved = missing
section = root.get(section_name, {})
if isinstance(section, dict) and field_name in section:
    resolved = section[field_name]

if resolved is missing:
    # Backward-compatible fallback: some callers pass section names that
    # belong to top-level sub-objects (for example command_samples.*).
    for candidate in root.values():
        if not isinstance(candidate, dict):
            continue
        nested_section = candidate.get(section_name, {})
        if isinstance(nested_section, dict) and field_name in nested_section:
            resolved = nested_section[field_name]
            break

if resolved is missing:
    print("")
    sys.exit(0)

emit_value(resolved, output_mode)
PY
}

_mfx_core_http_automation_first_catalog_process() {
    local file_path="$1"
    local resolved_exe
    resolved_exe="$(_mfx_core_http_automation_parse_first_key_value "$file_path" "exe" "string")"
    if [[ -n "$resolved_exe" ]]; then
        printf '%s\n' "$resolved_exe"
        return
    fi

    _mfx_core_http_automation_parse_first_key_value "$file_path" "process" "string"
}

_mfx_core_http_automation_parse_uint_field() {
    local input_file="$1"
    local field_name="$2"
    _mfx_core_http_automation_parse_first_key_value "$input_file" "$field_name" "uint"
}

_mfx_core_http_automation_parse_scalar_field() {
    local input_file="$1"
    local field_name="$2"
    _mfx_core_http_automation_parse_first_key_value "$input_file" "$field_name" "scalar"
}

_mfx_core_http_automation_parse_active_field() {
    local input_file="$1"
    local field_name="$2"
    _mfx_core_http_automation_parse_section_value "$input_file" "active" "$field_name" "string"
}

_mfx_core_http_automation_parse_section_scalar_field() {
    local input_file="$1"
    local section_name="$2"
    local field_name="$3"
    _mfx_core_http_automation_parse_section_value "$input_file" "$section_name" "$field_name" "scalar"
}

_mfx_core_http_automation_parse_nested_section_scalar_field() {
    local input_file="$1"
    local section_name="$2"
    local nested_section_name="$3"
    local field_name="$4"
    python3 - "$input_file" "$section_name" "$nested_section_name" "$field_name" <<'PY'
import json
import sys

input_file = sys.argv[1]
section_name = sys.argv[2]
nested_section_name = sys.argv[3]
field_name = sys.argv[4]

try:
    with open(input_file, "r", encoding="utf-8") as f:
        root = json.load(f)
except Exception:
    print("")
    sys.exit(0)

section = root.get(section_name, {})
resolved = None
if isinstance(section, dict):
    nested = section.get(nested_section_name, {})
    if isinstance(nested, dict) and field_name in nested:
        resolved = nested[field_name]

if resolved is None:
    # Backward-compatible fallback: many existing callers pass
    # section_name relative to a top-level bucket (for example
    # command_samples.hold.start => section_name=hold,nested=start).
    for candidate in root.values():
        if not isinstance(candidate, dict):
            continue
        section = candidate.get(section_name, {})
        if not isinstance(section, dict):
            continue
        nested = section.get(nested_section_name, {})
        if isinstance(nested, dict) and field_name in nested:
            resolved = nested[field_name]
            break

if resolved is None:
    print("")
    sys.exit(0)

value = resolved
if isinstance(value, bool):
    print("true" if value else "false")
elif isinstance(value, (int, float)):
    print(str(value))
elif value is None:
    print("null")
elif isinstance(value, str):
    print(json.dumps(value, ensure_ascii=False, separators=(",", ":")))
else:
    print(json.dumps(value, ensure_ascii=False, separators=(",", ":")))
PY
}

_mfx_core_http_automation_parse_first_mapping_scope_count() {
    local input_file="$1"
    python3 - "$input_file" <<'PY'
import json
import sys

with open(sys.argv[1], "r", encoding="utf-8") as f:
    root = json.load(f)

mappings = (
    root.get("automation", {})
        .get("mouse_mappings", [])
)
if not mappings:
    print("0")
    sys.exit(0)

scopes = mappings[0].get("app_scopes", [])
if not isinstance(scopes, list):
    print("0")
    sys.exit(0)

print(len(scopes))
PY
}

_mfx_core_http_automation_parse_first_mapping_scope_value() {
    local input_file="$1"
    python3 - "$input_file" <<'PY'
import json
import sys

with open(sys.argv[1], "r", encoding="utf-8") as f:
    root = json.load(f)

mappings = (
    root.get("automation", {})
        .get("mouse_mappings", [])
)
if not mappings:
    print("")
    sys.exit(0)

scopes = mappings[0].get("app_scopes", [])
if not isinstance(scopes, list) or not scopes:
    print("")
    sys.exit(0)

value = scopes[0]
print(value if isinstance(value, str) else "")
PY
}

_mfx_core_http_automation_parse_first_mapping_legacy_scope_value() {
    local input_file="$1"
    python3 - "$input_file" <<'PY'
import json
import sys

with open(sys.argv[1], "r", encoding="utf-8") as f:
    root = json.load(f)

mappings = (
    root.get("automation", {})
        .get("mouse_mappings", [])
)
if not mappings:
    print("")
    sys.exit(0)

value = mappings[0].get("app_scope", "")
print(value if isinstance(value, str) else "")
PY
}
