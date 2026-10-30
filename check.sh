cd ~/Projects/epiphany

git diff --check

meson setup build --reconfigure

set -o pipefail
ninja -C build -k0 2>&1 | tee build-all.log
