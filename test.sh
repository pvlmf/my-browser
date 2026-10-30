#!/usr/bin/env bash
set -Eeuo pipefail

ROOT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)"
cd "$ROOT_DIR"

BUILD_DIR="${BUILD_DIR:-build}"
LOG_FILE="${TEST_LOG:-$BUILD_DIR/logs/test.log}"

if [[ ! -f "$BUILD_DIR/build.ninja" ]]; then
  printf '[ERROR] Build directory is not configured: %s\n' "$BUILD_DIR" >&2
  printf 'Run: ./setup.sh\n' >&2
  exit 2
fi

# テスト専用のXDG設定ディレクトリを/tmpに作成
TEST_XDG_ROOT="$(mktemp -d "${TMPDIR:-/tmp}/epiphany-test-xdg.XXXXXX")"
trap 'rm -rf -- "$TEST_XDG_ROOT"' EXIT

mkdir -p \
    "$TEST_XDG_ROOT/home" \
    "$TEST_XDG_ROOT/config-dirs"

mkdir -p "$(dirname -- "$LOG_FILE")"
printf '[INFO] Running Meson tests in %s\n' "$BUILD_DIR"
printf '[INFO] Test log: %s\n\n' "$LOG_FILE"

if env \
    XDG_CONFIG_HOME="$TEST_XDG_ROOT/home" \
    XDG_CONFIG_DIRS="$TEST_XDG_ROOT/config-dirs" \
    meson test -C "$BUILD_DIR" --print-errorlogs "$@" 2>&1 | tee "$LOG_FILE"; then
  printf '\n[ ok ] Tests completed successfully\n'
else
  status=$?
  printf '\n========== TEST FAILURE SUMMARY ==========\n' >&2
  if ! grep -nE 'FAIL:|ERROR:|killed by signal|Summary of Failures|Fail:' "$LOG_FILE"; then
    printf 'No standard test failure pattern found. Last 80 log lines:\n' >&2
    tail -n 80 "$LOG_FILE" >&2
  fi
  printf '\nFull log: %s\n' "$LOG_FILE" >&2
  exit "$status"
fi
