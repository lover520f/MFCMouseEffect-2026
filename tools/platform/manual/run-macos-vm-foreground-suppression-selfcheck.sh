#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd -- "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd -- "$script_dir/../../.." && pwd)"

source "$repo_root/tools/platform/regression/lib/common.sh"
mfx_ok "macos vm foreground suppression selfcheck retired: VM foreground auto-suppression no longer exists; use effects blacklist coverage instead"
