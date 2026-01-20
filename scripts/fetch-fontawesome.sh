#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ASSETS_DIR="${ROOT_DIR}/assets/fonts"
TP_DIR="${ROOT_DIR}/third_party/fontawesome"
CACHE_DIR="${ROOT_DIR}/.cache/fontawesome"
TMP_DIR="${CACHE_DIR}/tmp"

mkdir -p "${ASSETS_DIR}" "${TP_DIR}" "${CACHE_DIR}"

echo "[FA] Resolving latest Font Awesome release..."
LATEST_JSON="$(curl -fsSL "https://api.github.com/repos/FortAwesome/Font-Awesome/releases/latest")"

TAG="$(python3 - <<'PY'
import json,sys
j=json.loads(sys.stdin.read())
print(j["tag_name"])
PY
<<<"${LATEST_JSON}")"

VER="${TAG#v}"

ZIP_NAME="fontawesome-free-${VER}-web.zip"
ZIP_URL="https://github.com/FortAwesome/Font-Awesome/releases/download/${TAG}/${ZIP_NAME}"
ZIP_PATH="${CACHE_DIR}/${ZIP_NAME}"

echo "[FA] Downloading ${ZIP_URL}"
curl -fL --retry 3 --retry-delay 2 -o "${ZIP_PATH}" "${ZIP_URL}"

rm -rf "${TMP_DIR}"
mkdir -p "${TMP_DIR}"
unzip -q -o "${ZIP_PATH}" -d "${TMP_DIR}"

BASE_DIR="$(find "${TMP_DIR}" -maxdepth 1 -type d -name "fontawesome-free-*-web" | head -n 1)"
if [[ -z "${BASE_DIR}" ]]; then
  echo "[FA] ERROR: Could not find extracted fontawesome-free-*-web directory."
  exit 1
fi

WEBFONTS_DIR="${BASE_DIR}/webfonts"

cp -f "${WEBFONTS_DIR}/fa-solid-900.ttf"  "${ASSETS_DIR}/"
cp -f "${WEBFONTS_DIR}/fa-brands-400.ttf" "${ASSETS_DIR}/"
if [[ -f "${WEBFONTS_DIR}/fa-regular-400.ttf" ]]; then
  cp -f "${WEBFONTS_DIR}/fa-regular-400.ttf" "${ASSETS_DIR}/"
fi

if [[ -f "${BASE_DIR}/LICENSE.txt" ]]; then
  cp -f "${BASE_DIR}/LICENSE.txt" "${TP_DIR}/LICENSE.txt"
fi

cat > "${TP_DIR}/SOURCE.txt" <<EOF
Font Awesome Free ${VER}
Downloaded from: ${ZIP_URL}
Contains: fa-solid-900.ttf, fa-brands-400.ttf (and optional fa-regular-400.ttf)
EOF

echo "[FA] Installed fonts into ${ASSETS_DIR}"
