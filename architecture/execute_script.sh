#!/usr/bin/env bash
set -euo pipefail

# Папка скрипта (чтобы запускать из любой директории)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Аргументы:
#   $1 = путь к .ptptb (если не задан — out_local.ptptb рядом со скриптом)
EXE="${1:-"$SCRIPT_DIR/out_local.ptptb"}"

# Настройки можно переопределять через env:
LOGIN="${LOGIN:-"507630"}"
PASSWORD="${PASSWORD:-"4647fcd6-dfe9-401b-b2ca-17958bf25079"}"

ARC_FILE="${ARC_FILE:-"$SCRIPT_DIR/twoaddr_dyn64_v10.target.pdsl"}"
ARC="${ARC:-"twoaddr_dyn64_v10"}"

CODE_BANK_NAME="${CODE_BANK_NAME:-"code"}"
EXEC_FLAGS="${EXEC_FLAGS:-"-il -w"}"

MANAGER=(mono "$SCRIPT_DIR/Portable.RemoteTasks.Manager.exe")

echo "Executing..."
echo "  EXE:        $EXE"
echo "  ARC_FILE:   $ARC_FILE"
echo "  ARCH:       $ARC"
echo "  CODE_BANK:  $CODE_BANK_NAME"

# Жёсткая защита от частой ошибки: передали .asm вместо .ptptb
if [[ "$EXE" != *.ptptb ]]; then
  echo "ERROR: expected a .ptptb binary to run, got: $EXE"
  echo "Hint: сначала собери asm в .ptptb, а сюда передавай именно .ptptb"
  exit 2
fi

# Проверим, что файлы реально существуют
[[ -f "$EXE" ]] || { echo "ERROR: binary not found: $EXE"; exit 3; }
[[ -f "$ARC_FILE" ]] || { echo "ERROR: architecture file not found: $ARC_FILE"; exit 4; }

"${MANAGER[@]}" -ul "$LOGIN" -up "$PASSWORD" $EXEC_FLAGS -s \
  ExecuteBinaryWithInteractiveInput definitionFile "$ARC_FILE" \
  archName "$ARC" binaryFileToRun "$EXE" \
  ipRegStorageName ip finishMnemonicName hlt \
  codeRamBankName "$CODE_BANK_NAME" \
  stdinRegStName rin stdoutRegStName rout

echo "Done."
