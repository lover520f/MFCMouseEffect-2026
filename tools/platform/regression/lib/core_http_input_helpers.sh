#!/usr/bin/env bash

set -euo pipefail

_mfx_core_http_input_helpers_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$_mfx_core_http_input_helpers_dir/core_http_input_parse_helpers.sh"
source "$_mfx_core_http_input_helpers_dir/core_http_input_permission_helpers.sh"
source "$_mfx_core_http_input_helpers_dir/core_http_input_notification_helpers.sh"
source "$_mfx_core_http_input_helpers_dir/core_http_input_state_helpers.sh"
source "$_mfx_core_http_input_helpers_dir/core_http_input_contract_steps.sh"
