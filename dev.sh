#!/usr/bin/env bash
set -Eeuo pipefail

ROOT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)"

"$ROOT_DIR/build.sh"
exec "$ROOT_DIR/run.sh" "$@"
