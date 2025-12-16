#!/usr/bin/env zsh
set -euo pipefail

ROOT=$(cd "$(dirname "$0")/.." && pwd)
cd "$ROOT"

# Build and run generator
clang -o Lab2/lab2_cfg Lab2/main.c Lab2/flow.c Lab1/src/parser.c vendor/tree-sitter/lib/src/lib.c \
  -I Lab1/src -I Lab2 -I vendor/tree-sitter/lib/include -I vendor/tree-sitter/lib/src -lm

./Lab2/generate_cfgs.sh

# Quick checks
OUT=Lab2/out
if [ ! -d "$OUT" ]; then
  echo "Output dir missing: $OUT"; exit 2
fi
# expect at least callgraph and all_functions
[ -f "$OUT/callgraph.dot" ] || { echo "callgraph.dot missing"; exit 3 }
[ -f "$OUT/all_functions.dot" ] || { echo "all_functions.dot missing"; exit 4 }

echo "Sanity checks passed: DOT files generated in $OUT"
