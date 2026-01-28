#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$ROOT"

# Credentials for the remote manager calls; override with environment variables if you need another account.
BUILD_LOGIN="${BUILD_LOGIN:-507630}"
BUILD_PASSWORD="${BUILD_PASSWORD:-4647fcd6-dfe9-401b-b2ca-17958bf25079}"

chmod +x build/build_script.sh build/execute_script.sh

BUILD_DIR="$ROOT/build"

echo "Running remote assemble via build/build_script.sh"
(
  cd "$BUILD_DIR"
  export LOGIN="$BUILD_LOGIN"
  export PASSWORD="$BUILD_PASSWORD"
  bash ./build_script.sh
)

echo "Running binary execution via build/execute_script.sh"
(
  cd "$BUILD_DIR"
  export LOGIN="$BUILD_LOGIN"
  export PASSWORD="$BUILD_PASSWORD"
  bash ./execute_script.sh
)
