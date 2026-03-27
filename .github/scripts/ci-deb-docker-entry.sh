#!/usr/bin/env bash
# Runs inside Docker (workspace mounted at /workspace). Called from GitHub Actions.
set -euo pipefail
cd /workspace

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
bash "$SCRIPT_DIR/ci-deb-setup.sh" "${OLD_RELEASE:-0}"

cmake -B build -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_WRAPPER_TESTS=OFF
cmake --build build -j"$(nproc)"

pushd build >/dev/null
ctest --output-on-failure
cpack -G DEB

DISTRO_LABEL="${DISTRO_LABEL:?missing DISTRO_LABEL}"
shopt -s nullglob
for f in *.deb; do
  base="${f%.deb}"
  mv -v "$f" "${base}_${DISTRO_LABEL}.deb"
done
ls -la *.deb
popd >/dev/null
