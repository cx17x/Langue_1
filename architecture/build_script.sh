#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

resolve_path() {
  local path="$1"
  case "$path" in
    /*) printf '%s\n' "$path" ;;
    *) printf '%s\n' "$SCRIPT_DIR/$path" ;;
  esac
}

ASM=${1:-${ASM_FILE:-""}}
if [[ -z "$ASM" ]]; then
  echo "Usage: $0 /path/to/listing.asm" >&2
  echo "Please provide an assembler listing (set ASM_FILE or pass it as the first argument)." >&2
  exit 1
fi

ASM=$(resolve_path "$ASM")
if [[ ! -f "$ASM" ]]; then
  echo "Assembler listing '$ASM' not found" >&2
  exit 1
fi

LOGIN=${LOGIN:-"507630"}
PASSWORD=${PASSWORD:-"4647fcd6-dfe9-401b-b2ca-17958bf25079"}
TARGET_FILE=${TARGET_FILE:-"$SCRIPT_DIR/twoaddr_dyn64_v10.target.pdsl"}
ARCH_NAME=${ARCH_NAME:-"twoaddr_dyn64_v10"}
SOURCES_DIR=${SOURCES_DIR:-"$SCRIPT_DIR"}
ASSEMBLE_LOG=${ASSEMBLE_LOG:-"$SCRIPT_DIR/assemble_remote.log"}
DOWNLOAD_LOG=${DOWNLOAD_LOG:-"$SCRIPT_DIR/download_remote.log"}

MANAGER=(mono "$SCRIPT_DIR/Portable.RemoteTasks.Manager.exe")

echo "Assembling..."

# ВАЖНО: забираем и stdout, и stderr, и показываем пользователю
ASSEMBLE_OUTPUT="$(
  "${MANAGER[@]}" \
    -ul "$LOGIN" \
    -up "$PASSWORD" \
    -s AssembleDebug \
    definitionFile "$TARGET_FILE" \
    archName "$ARCH_NAME" \
    asmListing "$ASM" \
    sourcesDir "$SOURCES_DIR" \
  2>&1 | tee "$ASSEMBLE_LOG"
)"

GUID="$(echo "$ASSEMBLE_OUTPUT" | grep -oE '[0-9a-fA-F]{8}-([0-9a-fA-F]{4}-){3}[0-9a-fA-F]{12}' | head -n1)"
if [[ -z "${GUID:-}" ]]; then
  echo "GUID not found. See assemble_remote.log"
  exit 1
fi
echo "GUID: $GUID"

echo "Downloading out.ptptb (retry)..."

# Несколько попыток: либо задача ещё не закончилась, либо файл появляется с задержкой
ok=0
for i in $(seq 1 20); do
  if "${MANAGER[@]}" -ul "$LOGIN" -up "$PASSWORD" -g "$GUID" -r "out.ptptb" -o "out_local.ptptb" 2>&1 | tee -a download_remote.log; then
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
