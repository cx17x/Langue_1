#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ASM=${1:-"$SCRIPT_DIR/test_twoaddr_dyn64_v10.asm"}
LOGIN=${LOGIN:-"507630"}
PASSWORD=${PASSWORD:-"4647fcd6-dfe9-401b-b2ca-17958bf25079"}

MANAGER=(mono "$SCRIPT_DIR/Portable.RemoteTasks.Manager.exe")

echo "Assembling..."

# Capture both stdout and stderr for debugging.
ASSEMBLE_OUTPUT="$(
  "${MANAGER[@]}" \
    -ul "$LOGIN" \
    -up "$PASSWORD" \
    -s AssembleDebug \
    definitionFile "$SCRIPT_DIR/twoaddr_dyn64_v10.target.pdsl" \
    archName "twoaddr_dyn64_v10" \
    asmListing "$ASM" \
    sourcesDir "$SCRIPT_DIR" \
  2>&1 | tee "$SCRIPT_DIR/assemble_remote.log"
)"

GUID="$(echo "$ASSEMBLE_OUTPUT" | grep -oE '[0-9a-fA-F]{8}-([0-9a-fA-F]{4}-){3}[0-9a-fA-F]{12}' | head -n1)"
if [[ -z "${GUID:-}" ]]; then
  echo "GUID not found. See assemble_remote.log"
  exit 1
fi
echo "GUID: $GUID"

echo "Downloading out.ptptb (retry)..."

ok=0
for i in $(seq 1 20); do
  if "${MANAGER[@]}" -ul "$LOGIN" -up "$PASSWORD" -g "$GUID" -r "out.ptptb" -o "$SCRIPT_DIR/out_local.ptptb" 2>&1 | tee -a "$SCRIPT_DIR/download_remote.log"; then
    ok=1
    break
  fi
  sleep 1
done

if [[ $ok -ne 1 ]]; then
  echo "Failed to download out.ptptb after retries."
  echo "Check assemble_remote.log and download_remote.log"
  exit 1
fi

echo "OK: out_local.ptptb downloaded"
