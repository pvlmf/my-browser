#!/usr/bin/env bash
set -Eeuo pipefail

ROOT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)"
cd "$ROOT_DIR"

BUILD_DIR="${BUILD_DIR:-build}"

if [[ ! -f "$BUILD_DIR/build.ninja" ]]; then
  printf '[ERROR] Build directory is not configured: %s\n' "$BUILD_DIR" >&2
  printf 'Run: ./setup.sh\n' >&2
  exit 2
fi

exec meson devenv -C "$BUILD_DIR" epiphany "$@"
