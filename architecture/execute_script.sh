#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
EXE=${1:-"$SCRIPT_DIR/out_local.ptptb"}
LOGIN=${LOGIN:-"507630"}
PASSWORD=${PASSWORD:-"4647fcd6-dfe9-401b-b2ca-17958bf25079"}
ARC_FILE=${ARC_FILE:-"$SCRIPT_DIR/twoaddr_dyn64_v10.target.pdsl"}
ARC=${ARC:-"twoaddr_dyn64_v10"}

MANAGER=(mono "$SCRIPT_DIR/Portable.RemoteTasks.Manager.exe")

echo "Executing..."

"${MANAGER[@]}" -ul "$LOGIN" -up "$PASSWORD" -il -w -s \
  ExecuteBinaryWithInteractiveInput definitionFile "$ARC_FILE" \
  archName "$ARC" binaryFileToRun "$EXE" \
  ipRegStorageName ip finishMnemonicName hlt \
  codeRamBankName codeMem stdinRegStName rin \
  stdoutRegStName rout

# Optional: use input redirection if the service supports it.
# Example:
# "${MANAGER[@]}" ... inputFile "$SCRIPT_DIR/in.txt" > "$SCRIPT_DIR/output.txt"
