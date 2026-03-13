#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/../.." && pwd)"
cd "$ROOT_DIR"

cmd="${1:-}"
shift || true

case "$cmd" in
  index)
    node ./tools/docs/ai-context-index.mjs "$@"
    ;;
  route)
    node ./tools/docs/ai-context-route.mjs "$@"
    ;;
  check)
    node ./tools/docs/ai-context-check.mjs "$@"
    ;;
  watch)
    node ./tools/docs/ai-context-watch.mjs "$@"
    ;;
  *)
    cat <<'EOF'
Usage:
  ./tools/docs/ai-context.sh index
  ./tools/docs/ai-context.sh route --task "automation gesture debug" [--budget 5200] [--top 8] [--json]
  ./tools/docs/ai-context.sh check [--strict] [--enforce-line-limits]
  ./tools/docs/ai-context.sh watch
EOF
    exit 1
    ;;
esac
