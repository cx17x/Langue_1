#!/usr/bin/env zsh
set -euo pipefail

# Скрипт собирает ast_dump (если нужно), парсит все примеры в Lab1/examples
# и генерирует PDF-файлы с деревьями (через graphviz dot).

ROOT="$(cd "$(dirname "$0")/.." && pwd)"

AST_DUMP="$ROOT/ast_dump"

build_ast_dump() {
  echo "Building ast_dump..."
  clang "$ROOT/ast_dump.c" "$ROOT/Lab1/src/parser.c" \
    "$ROOT/vendor/tree-sitter/lib/src/lib.c" \
    -I "$ROOT/vendor/tree-sitter/lib/include" -I "$ROOT/vendor/tree-sitter/lib/src" -o "$AST_DUMP"
}

if [ ! -x "$AST_DUMP" ]; then
  build_ast_dump
fi

if [ ! -d "$ROOT/Lab1/examples" ]; then
  echo "No examples directory: $ROOT/Lab1/examples" >&2
  exit 1
fi

for f in "$ROOT"/Lab1/examples/*.txt; do
  [ -e "$f" ] || continue
  base=$(basename "$f" .txt)
  dotfile="$ROOT/Lab1/examples/${base}.dot"
  pdffile="$ROOT/Lab1/examples/${base}.pdf"

  echo "Parsing $f -> $dotfile"
  "$AST_DUMP" "$f" "$dotfile"

  if command -v dot >/dev/null 2>&1; then
    echo "Generating PDF $pdffile"
    dot -Tpdf "$dotfile" -o "$pdffile"
  else
    echo "Warning: 'dot' (graphviz) not found; skipping PDF for $base"
  fi
done

echo "All done. PDFs are in Lab1/examples/*.pdf (when dot is available)."
