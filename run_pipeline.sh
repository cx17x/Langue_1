#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$SCRIPT_DIR"
OUTDIR="$ROOT/Lab2/out"

echo "[1/3] Building lab2_cfg..."
clang "$ROOT/Lab2/main.c" "$ROOT/Lab2/flow.c" "$ROOT/Lab1/src/parser.c" \
  "$ROOT/vendor/tree-sitter/lib/src/lib.c" \
  -I "$ROOT/vendor/tree-sitter/lib/include" \
  -I "$ROOT/vendor/tree-sitter/lib/src" \
  -o "$ROOT/lab2_cfg"

mkdir -p "$OUTDIR"

echo "[2/3] Running lab2_cfg on Lab1/examples ..."
"$ROOT/lab2_cfg" "$ROOT"/Lab1/examples/*.txt --outdir "$OUTDIR"

# remove legacy expression graphs
rm -f "$OUTDIR"/*.expr.dot "$OUTDIR"/*.expr.pdf 2>/dev/null || true

echo "[3/3] Converting DOT → PDF..."
if ! command -v dot >/dev/null 2>&1; then
  echo "Graphviz 'dot' not found — skipping PDF conversion." >&2
  exit 0
fi

shopt -s nullglob
for dot_file in "$OUTDIR"/*.dot; do
  pdf_file="${dot_file%.dot}.pdf"
  dot -Tpdf "$dot_file" -o "$pdf_file"
  echo "Generated $pdf_file"
done
