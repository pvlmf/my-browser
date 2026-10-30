#!/usr/bin/env bash
set -Eeuo pipefail

ROOT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)"
cd "$ROOT_DIR"

BUILD_DIR="${BUILD_DIR:-build}"

if [[ -f "$BUILD_DIR/build.ninja" ]]; then
  printf '[INFO] Reconfiguring existing build: %s\n' "$BUILD_DIR"
  meson configure "$BUILD_DIR" -Ddeveloper_mode=true "$@"
else
  printf '[INFO] Creating debug build: %s\n' "$BUILD_DIR"
  meson setup "$BUILD_DIR" --buildtype=debug -Ddeveloper_mode=true "$@"
fi
