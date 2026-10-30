#!/usr/bin/env bash
set -Eeuo pipefail

ROOT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)"
cd "$ROOT_DIR"

BUILD_DIR="${BUILD_DIR:-build}"
LOG_FILE="${LOG_FILE:-build-all.log}"

# 必要なコマンドの確認
for cmd in git meson ninja tee grep; do
    if ! command -v "$cmd" >/dev/null 2>&1; then
        printf '[ERROR] Required command not found: %s\n' "$cmd" >&2
        exit 127
    fi
done

# チェックを実行し、失敗した段階で終了する
run_check() {
    local label="$1"
    shift

    printf '\n========== %s ==========\n' "$label"

    if "$@"; then
        printf '[ OK ] %s\n' "$label"
    else
        local status=$?
        printf '[FAIL] %s (exit code: %d)\n' "$label" "$status" >&2
        exit "$status"
    fi
}

# 1. シェルスクリプトの構文確認
run_check "Shell syntax" bash -n "$0"

# 2. ShellCheck（インストール済みの場合のみ）
if command -v shellcheck >/dev/null 2>&1; then
    run_check "ShellCheck" shellcheck "$0"
else
    printf '[SKIP] ShellCheck is not installed\n'
fi

# 3. Git差分の空白チェック
run_check "Git diff check" git diff --check HEAD --

# 4. ビルド環境の確認
if [[ ! -f "$BUILD_DIR/build.ninja" ]]; then
    printf '[ERROR] Build directory is not configured: %s\n' "$BUILD_DIR" >&2
    printf 'Run: meson setup %s\n' "$BUILD_DIR" >&2
    exit 1
fi

# 5. Mesonの再設定
run_check "Meson reconfigure" meson setup "$BUILD_DIR" --reconfigure

# 6. ビルド
printf '\n========== Build ==========\n'
printf 'Build directory: %s\n' "$BUILD_DIR"
printf 'Log file: %s\n' "$LOG_FILE"

if ninja -C "$BUILD_DIR" -k0 2>&1 | tee "$LOG_FILE"; then
    printf '\n[ OK ] Build completed successfully\n'
else
    build_status=$?

    printf '\n========== BUILD ERROR SUMMARY ==========\n'

    if ! grep -nE \
        'FAILED:|fatal error:|error:|undefined reference|collect2:|ninja: build stopped' \
        "$LOG_FILE"; then
        printf 'No recognized error lines. Last 80 lines:\n'
        tail -n 80 "$LOG_FILE"
    fi

    printf '\nFull log: %s\n' "$LOG_FILE"
    exit "$build_status"
fi

printf '\n========== ALL CHECKS PASSED ==========\n'
