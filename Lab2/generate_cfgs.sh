#!/usr/bin/env zsh
set -euo pipefail

ROOT=$(cd "$(dirname "$0")/.." && pwd)
OUTDIR="$ROOT/Lab2/out"
mkdir -p "$OUTDIR"

# clean previous outputs to avoid stale per-function artifacts
rm -f "$OUTDIR"/*.dot "$OUTDIR"/*.pdf || true

LAB2_BIN="$ROOT/lab2_cfg"
if [ ! -x "$LAB2_BIN" ]; then
  echo "Building lab2_cfg..."
  clang "$ROOT/Lab2/main.c" "$ROOT/Lab1/src/parser.c" \
    "$ROOT/vendor/tree-sitter/lib/src/lib.c" -I "$ROOT/vendor/tree-sitter/lib/include" -I "$ROOT/vendor/tree-sitter/lib/src" -o "$LAB2_BIN"
fi

if [ $# -eq 0 ]; then
  files=("$ROOT/Lab1/examples"/*.txt)
else
  files=("$@")
fi

echo "Generating CFG DOTs into $OUTDIR"
"$LAB2_BIN" "${files[@]}" --outdir "$OUTDIR"

echo "Assembling global all_functions.dot"
ALLF="$OUTDIR/all_functions.dot"
echo "digraph AllFunctions {" > "$ALLF"
for f in "$OUTDIR"/*.dot; do
  bn=$(basename "$f")
  # skip callgraph and existing all_functions
  if [ "$bn" = "callgraph.dot" ] || [ "$bn" = "all_functions.dot" ]; then
    continue
  fi
  # append body without first and last line
  sed '1d;$d' "$f" >> "$ALLF"
done
echo "}" >> "$ALLF"

echo "Done. DOT files are in $OUTDIR"

echo "Converting DOT -> PDF"
for f in "$OUTDIR"/*.dot; do
  dot -Tpdf "$f" -o "${f%.dot}.pdf" && echo "Wrote ${f%.dot}.pdf"
done
