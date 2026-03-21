#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/../.." && pwd)"
HOOK_PATH="$ROOT_DIR/.git/hooks/pre-commit"

cat > "$HOOK_PATH" <<'EOF'
#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(git rev-parse --show-toplevel)"
cd "$ROOT_DIR"

./tools/docs/ai-context.sh index
# Stage regenerated artifacts into the same commit so the worktree
# does not appear dirty immediately after commit.
git add docs/.ai/context-index.json docs/.ai/context-map.md
./tools/docs/ai-context.sh check --strict
EOF

chmod +x "$HOOK_PATH"
echo "[ok] installed pre-commit hook: $HOOK_PATH"
