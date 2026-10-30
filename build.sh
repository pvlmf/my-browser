#!/usr/bin/env bash
set -Eeuo pipefail

ROOT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)"
cd "$ROOT_DIR"

BUILD_DIR="${BUILD_DIR:-build}"
JOBS="${JOBS:-1}"
LOG_FILE="${BUILD_LOG:-$BUILD_DIR/logs/build.log}"

if [[ ! -f "$BUILD_DIR/build.ninja" ]]; then
  printf '[ERROR] Build directory is not configured: %s\n' "$BUILD_DIR" >&2
  printf 'Run: ./setup.sh\n' >&2
  exit 2
fi

if [[ ! "$JOBS" =~ ^[1-9][0-9]*$ ]]; then
  printf '[ERROR] JOBS must be a positive integer (got: %s)\n' "$JOBS" >&2
  exit 2
fi

mkdir -p "$(dirname -- "$LOG_FILE")"
printf '[INFO] Building in %s (jobs: %s)\n' "$BUILD_DIR" "$JOBS"
printf '[INFO] Build log: %s\n\n' "$LOG_FILE"

if meson compile -C "$BUILD_DIR" --jobs "$JOBS" "$@" 2>&1 | tee "$LOG_FILE"; then
  printf '\n[ OK ] Build completed successfully\n'
else
  status=$?
  printf '\n========== BUILD ERROR SUMMARY ==========\n' >&2
  if ! grep -nE 'FAILED:|fatal error:|error:|undefined reference|collect2:|ninja: build stopped' "$LOG_FILE"; then
    printf 'No standard compiler error pattern found. Last 80 log lines:\n' >&2
    tail -n 80 "$LOG_FILE" >&2
  fi
  printf '\nFull log: %s\n' "$LOG_FILE" >&2
  exit "$status"
fi
