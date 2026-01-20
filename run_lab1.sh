#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
GEN_SCRIPT="$SCRIPT_DIR/Lab1/generate_trees.sh"

if [[ ! -x "$GEN_SCRIPT" ]]; then
  chmod +x "$GEN_SCRIPT"
fi

echo "[Lab1] Rebuilding AST dumps and PDFs..."
"$GEN_SCRIPT"
echo "[Lab1] Done."
