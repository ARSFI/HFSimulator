#!/usr/bin/env bash
set -euo pipefail

readonly EXPECTED_CLI_VERSION="1.5.1"
readonly EXPECTED_CLI_SHA256="cbb47ec4742ee49854031728a0eaeb678a2b36d7a797ef833a1a5b02062149c6"
readonly EXPECTED_CORE_VERSION="1.62.0"
readonly EXPECTED_BOARDS_SHA256="b58e03c4ccfd5a6f471ba05350987e0f845d0885255dd26fe163422d67c48714"
readonly EXPECTED_PLATFORM_SHA256="24d373f3a2fa4efd96b5980757304e4753443115d4d6c798af12a73a7566217a"
readonly EXPECTED_COMPILE_VERSION="15.2.1"
readonly FQBN="teensy:avr:teensy40:usb=serial,speed=600,opt=o2std,keys=en-us"

if [[ $# -ne 2 ]]; then
  echo "usage: $0 YYYYMMDDTHHMMSSZ OUTPUT_DIRECTORY" >&2
  exit 2
fi

readonly BUILD_UTC="$1"
readonly OUTPUT_DIRECTORY="$2"
[[ "${BUILD_UTC}" =~ ^[0-9]{8}T[0-9]{6}Z$ ]] || { echo "invalid build UTC" >&2; exit 2; }

readonly REPOSITORY_ROOT="$(git rev-parse --show-toplevel)"
readonly ARDUINO_CLI_BIN="${ARDUINO_CLI_BIN:-arduino-cli}"
readonly ARDUINO_DATA_DIRECTORY="${ARDUINO_DATA_DIRECTORY:-${HOME}/.arduino15}"
readonly PLATFORM_DIRECTORY="${ARDUINO_DATA_DIRECTORY}/packages/teensy/hardware/avr/${EXPECTED_CORE_VERSION}"
readonly CLI_PATH="$(command -v "${ARDUINO_CLI_BIN}")"
readonly GIT_DESCRIBE="$(git -C "${REPOSITORY_ROOT}" describe --always --long)"

[[ -z "$(git -C "${REPOSITORY_ROOT}" status --porcelain --untracked-files=no)" ]] || {
  echo "tracked source tree is dirty" >&2
  exit 1
}
[[ "$(sha256sum "${CLI_PATH}" | awk '{print $1}')" == "${EXPECTED_CLI_SHA256}" ]] || {
  echo "arduino-cli SHA-256 mismatch" >&2
  exit 1
}
"${CLI_PATH}" version | grep -F "Version: ${EXPECTED_CLI_VERSION}" >/dev/null
[[ "$(sha256sum "${PLATFORM_DIRECTORY}/boards.txt" | awk '{print $1}')" == "${EXPECTED_BOARDS_SHA256}" ]]
[[ "$(sha256sum "${PLATFORM_DIRECTORY}/platform.txt" | awk '{print $1}')" == "${EXPECTED_PLATFORM_SHA256}" ]]
[[ -d "${ARDUINO_DATA_DIRECTORY}/packages/teensy/tools/teensy-compile/${EXPECTED_COMPILE_VERSION}" ]]
for tool in teensy-tools teensy-discovery teensy-monitor; do
  [[ -d "${ARDUINO_DATA_DIRECTORY}/packages/teensy/tools/${tool}/${EXPECTED_CORE_VERSION}" ]]
done

mkdir -p "${OUTPUT_DIRECTORY}"
readonly OUTPUT_ABSOLUTE="$(cd "${OUTPUT_DIRECTORY}" && pwd)"
readonly STAGING_ROOT="$(mktemp -d /dev/shm/hfsim-teensy40-repro.XXXXXX)"
trap 'rm -rf -- "${STAGING_ROOT}"' EXIT
readonly WORK_DIRECTORY="${STAGING_ROOT}/work"
readonly BUILD_DIRECTORY="${STAGING_ROOT}/build"
readonly EXPORT_DIRECTORY="${STAGING_ROOT}/export"
readonly BUILD_UTC_ISO="${BUILD_UTC:0:4}-${BUILD_UTC:4:2}-${BUILD_UTC:6:2} ${BUILD_UTC:9:2}:${BUILD_UTC:11:2}:${BUILD_UTC:13:2} UTC"
export SOURCE_DATE_EPOCH="$(date -u -d "${BUILD_UTC_ISO}" +%s)"

write_identity_header() {
  local sketch_directory="$1"
  {
    echo '#pragma once'
    echo '#define HFSIM_RELEASE_BUILD 1'
    echo '#define HFSIM_BUILD_IDENTITY_ATTESTED 1'
    printf '#define HFSIM_GIT_DESCRIBE "%s"\n' "${GIT_DESCRIBE}"
    printf '#define HFSIM_BUILD_UTC "%s"\n' "${BUILD_UTC}"
  } > "${sketch_directory}/BuildIdentity.h"
}

write_manifest() {
  local destination="$1"
  {
    printf 'source_commit=%s\n' "$(git -C "${REPOSITORY_ROOT}" rev-parse HEAD)"
    printf 'source_tree=%s\n' "$(git -C "${REPOSITORY_ROOT}" rev-parse HEAD^{tree})"
    printf 'git_describe=%s\n' "${GIT_DESCRIBE}"
    printf 'build_utc=%s\n' "${BUILD_UTC}"
    printf 'source_date_epoch=%s\n' "${SOURCE_DATE_EPOCH}"
    printf 'fqbn=%s\n' "${FQBN}"
    printf 'arduino_cli_version=%s\n' "${EXPECTED_CLI_VERSION}"
    printf 'arduino_cli_sha256=%s\n' "${EXPECTED_CLI_SHA256}"
    printf 'teensy_core_version=%s\n' "${EXPECTED_CORE_VERSION}"
    printf 'boards_txt_sha256=%s\n' "${EXPECTED_BOARDS_SHA256}"
    printf 'platform_txt_sha256=%s\n' "${EXPECTED_PLATFORM_SHA256}"
    printf 'teensy_compile_version=%s\n' "${EXPECTED_COMPILE_VERSION}"
    printf 'audio_tree=%s\n' "$(git -C "${REPOSITORY_ROOT}" rev-parse HEAD:src/libraries/Audio)"
    printf 'bounce2_tree=%s\n' "$(git -C "${REPOSITORY_ROOT}" rev-parse HEAD:src/libraries/Bounce2)"
    printf 'ili9341_t3_tree=%s\n' "$(git -C "${REPOSITORY_ROOT}" rev-parse HEAD:src/libraries/ILI9341_t3)"
    printf 'encoder_tree=%s\n' "$(git -C "${REPOSITORY_ROOT}" rev-parse HEAD:src/libraries/Encoder)"
    printf 'encoder2_h_sha256=%s\n' "$(git -C "${REPOSITORY_ROOT}" show HEAD:src/HFSim_BFD_2_05/Encoder2.h | sha256sum | awk '{print $1}')"
  } > "${destination}"
}

compile_once() {
  local run_name="$1"
  rm -rf -- "${WORK_DIRECTORY}" "${BUILD_DIRECTORY}" "${EXPORT_DIRECTORY}"
  mkdir -p "${WORK_DIRECTORY}" "${BUILD_DIRECTORY}" "${EXPORT_DIRECTORY}"
  git -C "${REPOSITORY_ROOT}" archive --format=tar HEAD | tar -xf - -C "${WORK_DIRECTORY}"
  cp -a "${WORK_DIRECTORY}/src/libraries/Encoder/utility" "${WORK_DIRECTORY}/src/HFSim_BFD_2_05/utility"
  write_identity_header "${WORK_DIRECTORY}/src/HFSim_BFD_2_05"

  local run_output="${OUTPUT_ABSOLUTE}/${run_name}"
  mkdir -p "${run_output}/build" "${run_output}/artifacts"
  cp "${WORK_DIRECTORY}/src/HFSim_BFD_2_05/BuildIdentity.h" "${run_output}/BuildIdentity.h"
  write_manifest "${run_output}/build-manifest.txt"

  "${CLI_PATH}" compile \
    --fqbn "${FQBN}" \
    --build-path "${BUILD_DIRECTORY}" \
    --output-dir "${EXPORT_DIRECTORY}" \
    --library "${WORK_DIRECTORY}/src/libraries/Audio" \
    --library "${WORK_DIRECTORY}/src/libraries/Bounce2" \
    --library "${WORK_DIRECTORY}/src/libraries/ILI9341_t3" \
    --warnings all \
    --verbose \
    "${WORK_DIRECTORY}/src/HFSim_BFD_2_05" \
    2>&1 | tee "${run_output}/compile.log"

  cp -a "${BUILD_DIRECTORY}/." "${run_output}/build/"
  cp -a "${EXPORT_DIRECTORY}/." "${run_output}/artifacts/"
  (cd "${run_output}" && find . -type f ! -name SHA256SUMS -print0 | sort -z | xargs -0 sha256sum) > "${run_output}/SHA256SUMS"
}

compile_once run1
compile_once run2

readonly HEX1="$(find "${OUTPUT_ABSOLUTE}/run1/artifacts" -maxdepth 1 -name '*.hex' -type f -print -quit)"
readonly HEX2="$(find "${OUTPUT_ABSOLUTE}/run2/artifacts" -maxdepth 1 -name '*.hex' -type f -print -quit)"
[[ -n "${HEX1}" && -n "${HEX2}" ]]
cmp --silent "${HEX1}" "${HEX2}" || { echo "HEX reproducibility check failed" >&2; exit 1; }
sha256sum "${HEX1}" | tee "${OUTPUT_ABSOLUTE}/HEX_SHA256"
printf '%s@%s\n' "${GIT_DESCRIBE}" "${BUILD_UTC}" | tee "${OUTPUT_ABSOLUTE}/VERSION_STRING"
echo "reproducible HEX confirmed"
