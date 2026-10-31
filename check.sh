#!/usr/bin/env bash
set -Eeuo pipefail

ROOT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)"
cd "$ROOT_DIR"

BUILD_DIR="${BUILD_DIR:-build}"
SCRIPTS=(build.sh check.sh setup.sh run.sh dev.sh test.sh)

printf '\n========== Shell syntax ==========\n'
for script in "${SCRIPTS[@]}"; do
  bash -n "$ROOT_DIR/$script"
  printf '[ OK ] %s\n' "$script"
done

if command -v shellcheck >/dev/null 2>&1; then
  printf '\n========== ShellCheck ==========\n'
  shellcheck "${SCRIPTS[@]}"
  printf '[ OK ] ShellCheck\n'
else
  printf '\n[SKIP] ShellCheck is not installed\n'
fi

printf '\n========== C\Blueprint formatting ==========\n'
bash "$ROOT_DIR/data/check-code-style"
printf '[ OK ] C\Blueprint formatting\n'

printf '\n========== Git diff check ==========\n'
git diff --check HEAD --
printf '[ OK ] Git diff check\n'

if [[ ! -f "$BUILD_DIR/build.ninja" ]]; then
  printf '\n[ERROR] Build directory is not configured: %s\n' "$BUILD_DIR" >&2
  printf 'Run: ./setup.sh\n' >&2
  exit 2
fi

printf '\n========== Meson reconfigure ==========\n'
meson setup "$BUILD_DIR" --reconfigure -Ddeveloper_mode=true
printf '[ OK ] Meson reconfigure\n'

printf '\n========== Compile ==========\n'
"$ROOT_DIR/build.sh"

printf '\n========== ALL CHECKS PASSED ==========\n'
