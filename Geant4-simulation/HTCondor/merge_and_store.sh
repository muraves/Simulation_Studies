#!/usr/bin/env bash
# =============================================================================
# merge_and_store.sh
#
# Merges per-job .root files with hadd, auto-numbers the run, then copies
# all outputs into the canonical storage layout under:
#   /pnfs/iihe/muraves/muraves_SIM/${OUTPUT_SUBDIR}
#
# Usage:
#   ./merge_and_store.sh --subdir <NAME/DET_NAME> --cluster <id> [OPTIONS]
#
# Options:
#   --subdir   <NAME/DET_NAME>  Required. e.g. "MARIAELENA/NERO"
#   --cluster  <id>             Required. HTCondor ClusterId, e.g. 2612078
#   --flag     <flag>           Optional. filename flag suffix (default: test)
#   --rootdir  <path>           Dir containing musimData_*.root  (default: .)
#   --logdir   <path>           Dir containing musimLog_*.txt    (default: .)
#   --htcdir   <path>           Dir containing HTCondor files    (default: .)
#   --tmpdir   <path>           Scratch dir for the merged file  (default: .)
#   --dryrun                    Print what would happen; do nothing
# =============================================================================
set -euo pipefail

# --------------------------------------------------------------------------- #
# Helpers
# --------------------------------------------------------------------------- #
log()  { echo "[INFO]  $*"; }
warn() { echo "[WARN]  $*" >&2; }
die()  { echo "[ERROR] $*" >&2; exit 1; }

do_cp() {
    # Plain cp, no -p: avoids metadata errors on dCache write-only mount
    local src="$1" dst="$2"
    if [[ "${DRYRUN}" == "1" ]]; then
        echo "  [dryrun] cp \"${src}\" \"${dst}\""
    else
        cp "${src}" "${dst}"
    fi
}

do_mkdir() {
    local dir="$1"
    if [[ "${DRYRUN}" == "1" ]]; then
        echo "  [dryrun] mkdir -p \"${dir}\""
    else
        mkdir -p "${dir}"
    fi
}

# --------------------------------------------------------------------------- #
# Defaults
# --------------------------------------------------------------------------- #
OUTPUT_SUBDIR=""
CLUSTER_ID=""
FLAG="test"
ROOT_DIR="."
LOG_DIR="."
HTC_DIR="."
TMP_DIR="."
DRYRUN=0

PNFS_BASE="/pnfs/iihe/muraves/muraves_SIM"

# --------------------------------------------------------------------------- #
# Argument parsing
# --------------------------------------------------------------------------- #
while [[ $# -gt 0 ]]; do
    case "$1" in
        --subdir)   OUTPUT_SUBDIR="$2"; shift 2 ;;
        --cluster)  CLUSTER_ID="$2";    shift 2 ;;
        --flag)     FLAG="$2";          shift 2 ;;
        --rootdir)  ROOT_DIR="$2";      shift 2 ;;
        --logdir)   LOG_DIR="$2";       shift 2 ;;
        --htcdir)   HTC_DIR="$2";       shift 2 ;;
        --tmpdir)   TMP_DIR="$2";       shift 2 ;;
        --dryrun)   DRYRUN=1;           shift   ;;
        *) die "Unknown option: $1" ;;
    esac
done

[[ -z "${OUTPUT_SUBDIR}" ]] && die "--subdir is required (e.g. MARIAELENA/NERO)"
[[ -z "${CLUSTER_ID}"    ]] && die "--cluster is required (e.g. 2612078)"

OUTPUT_SUBDIR="${OUTPUT_SUBDIR#/}"
DEST_BASE="${PNFS_BASE}/${OUTPUT_SUBDIR}"

log "Cluster ID : ${CLUSTER_ID}"
log "Flag       : ${FLAG}"
log "Destination: ${DEST_BASE}"

# --------------------------------------------------------------------------- #
# Collect input files — filtered by ClusterId and flag
#
# Expected naming conventions:
#   musimData_c<id>_p*_<flag>.root
#   musimLog_c<id>_p*_<flag>.txt
#   <id>_<procid>.out
#   <id>_<procid>.err
#   <id>.log
# --------------------------------------------------------------------------- #
mapfile -t ROOT_FILES < <(find "${ROOT_DIR}" -maxdepth 1 \
    -name "musimData_c${CLUSTER_ID}_p*_${FLAG}.root" | sort)

mapfile -t LOG_FILES  < <(find "${LOG_DIR}"  -maxdepth 1 \
    -name "musimLog_c${CLUSTER_ID}_p*_${FLAG}.txt"   | sort)

mapfile -t HTC_OUT    < <(find "${HTC_DIR}"  -maxdepth 1 \
    -name "${CLUSTER_ID}_*.out" | sort)

mapfile -t HTC_ERR    < <(find "${HTC_DIR}"  -maxdepth 1 \
    -name "${CLUSTER_ID}_*.err" | sort)

# The HTCondor cluster log is a single file: <ClusterId>.log
HTC_LOG_FILE="${HTC_DIR}/${CLUSTER_ID}.log"

# --------------------------------------------------------------------------- #
# Report what was found
# --------------------------------------------------------------------------- #
log "Found ${#ROOT_FILES[@]} .root file(s)  (musimData_c${CLUSTER_ID}_p*_${FLAG}.root)"
log "Found ${#LOG_FILES[@]}  .txt  file(s)  (musimLog_c${CLUSTER_ID}_p*_${FLAG}.txt)"
log "Found ${#HTC_OUT[@]}    .out  file(s)  (${CLUSTER_ID}_*.out)"
log "Found ${#HTC_ERR[@]}    .err  file(s)  (${CLUSTER_ID}_*.err)"
if [[ -f "${HTC_LOG_FILE}" ]]; then
    log "Found HTCondor .log             (${CLUSTER_ID}.log)"
else
    warn "HTCondor .log not found at ${HTC_LOG_FILE} -- will skip"
fi

[[ ${#ROOT_FILES[@]} -eq 0 ]] && die "No matching .root files found — check --cluster and --flag."

# --------------------------------------------------------------------------- #
# Determine next run number
# --------------------------------------------------------------------------- #
next_run_number() {
    local base="$1"
    local max=0 n

    if ls "${base}"/run_[0-9]* &>/dev/null 2>&1; then
        for d in "${base}"/run_[0-9]*; do
            [[ -d "${d}" ]] || continue
            n="${d##*/run_}"
            [[ "${n}" =~ ^[0-9]+$ ]] && (( n > max )) && max="${n}"
        done
    elif command -v gfal-ls &>/dev/null; then
        while IFS= read -r entry; do
            [[ "${entry}" =~ ^run_([0-9]+)$ ]] && \
                (( ${BASH_REMATCH[1]} > max )) && max="${BASH_REMATCH[1]}"
        done < <(gfal-ls "gsiftp://dcache-door.iihe.ac.be/${base}" 2>/dev/null || true)
    fi

    echo $(( max + 1 ))
}

RUN_ID=$(next_run_number "${DEST_BASE}")
MERGED_NAME="musimData_run${RUN_ID}.root"
MERGED_PATH="${TMP_DIR}/${MERGED_NAME}"

log "Run number : ${RUN_ID} -> ${MERGED_NAME}"

# --------------------------------------------------------------------------- #
# Merge with hadd
# --------------------------------------------------------------------------- #
command -v hadd &>/dev/null || die "'hadd' not found."

log "Running hadd ..."
if [[ "${DRYRUN}" == "1" ]]; then
    echo "  [dryrun] hadd -f \"${MERGED_PATH}\" ${ROOT_FILES[*]}"
else
    hadd -f "${MERGED_PATH}" "${ROOT_FILES[@]}"
fi

# --------------------------------------------------------------------------- #
# Build destination directory tree
# --------------------------------------------------------------------------- #
RUN_DIR="${DEST_BASE}/run_${RUN_ID}"

# Change this to whatever name you prefer for the per-job root files subdir
INDIVIDUAL_ROOT_SUBDIR="root_files"

DIR_MERGED_ROOT="${DEST_BASE}"
DIR_INDIV_ROOT="${RUN_DIR}/${INDIVIDUAL_ROOT_SUBDIR}"
DIR_RUN_LOGS="${RUN_DIR}/run_logs"
DIR_HTC_LOG="${RUN_DIR}/HTCondor/log"
DIR_HTC_OUT="${RUN_DIR}/HTCondor/out"
DIR_HTC_ERR="${RUN_DIR}/HTCondor/err"

for d in "${DIR_MERGED_ROOT}" "${DIR_INDIV_ROOT}" \
          "${DIR_RUN_LOGS}" \
          "${DIR_HTC_LOG}" "${DIR_HTC_OUT}" "${DIR_HTC_ERR}"; do
    do_mkdir "${d}"
done

# --------------------------------------------------------------------------- #
# Copy files
# --------------------------------------------------------------------------- #
log "Copying merged .root -> ${DIR_MERGED_ROOT}/"
do_cp "${MERGED_PATH}" "${DIR_MERGED_ROOT}/${MERGED_NAME}"

log "Copying individual .root files -> ${DIR_INDIV_ROOT}/"
for f in "${ROOT_FILES[@]}"; do
    do_cp "${f}" "${DIR_INDIV_ROOT}/$(basename "${f}")"
done

if [[ ${#LOG_FILES[@]} -gt 0 ]]; then
    log "Copying run logs -> ${DIR_RUN_LOGS}/"
    for f in "${LOG_FILES[@]}"; do
        do_cp "${f}" "${DIR_RUN_LOGS}/$(basename "${f}")"
    done
else
    warn "No matching .txt log files -- skipping run_logs copy."
fi

if [[ -f "${HTC_LOG_FILE}" ]]; then
    log "Copying HTCondor .log -> ${DIR_HTC_LOG}/"
    do_cp "${HTC_LOG_FILE}" "${DIR_HTC_LOG}/$(basename "${HTC_LOG_FILE}")"
fi

if [[ ${#HTC_OUT[@]} -gt 0 ]]; then
    log "Copying HTCondor .out -> ${DIR_HTC_OUT}/"
    for f in "${HTC_OUT[@]}"; do
        do_cp "${f}" "${DIR_HTC_OUT}/$(basename "${f}")"
    done
fi

if [[ ${#HTC_ERR[@]} -gt 0 ]]; then
    log "Copying HTCondor .err -> ${DIR_HTC_ERR}/"
    for f in "${HTC_ERR[@]}"; do
        do_cp "${f}" "${DIR_HTC_ERR}/$(basename "${f}")"
    done
fi

# --------------------------------------------------------------------------- #
# Remove local source files now that everything is safely on /pnfs
# --------------------------------------------------------------------------- #
if [[ "${DRYRUN}" == "0" ]]; then
    log "Removing local source files ..."

    for f in "${ROOT_FILES[@]}"; do rm -f "${f}" && log "  removed ${f}"; done
    for f in "${LOG_FILES[@]}";  do rm -f "${f}" && log "  removed ${f}"; done
    [[ -f "${HTC_LOG_FILE}" ]]   && rm -f "${HTC_LOG_FILE}" && log "  removed ${HTC_LOG_FILE}"
    for f in "${HTC_OUT[@]}";    do rm -f "${f}" && log "  removed ${f}"; done
    for f in "${HTC_ERR[@]}";    do rm -f "${f}" && log "  removed ${f}"; done

    # Remove the local merged file from tmpdir (already copied to /pnfs)
    rm -f "${MERGED_PATH}" && log "  removed merged tmp file ${MERGED_PATH}"
else
    echo "  [dryrun] would remove: ${#ROOT_FILES[@]} .root, ${#LOG_FILES[@]} .txt, ${#HTC_OUT[@]} .out, ${#HTC_ERR[@]} .err, HTCondor .log, merged tmp file"
fi

# --------------------------------------------------------------------------- #
# Append to local manifest
# --------------------------------------------------------------------------- #
MANIFEST="${TMP_DIR}/manifest.log"
if [[ "${DRYRUN}" == "0" ]]; then
    echo "$(date '+%Y-%m-%d %H:%M:%S')  run_${RUN_ID}  cluster=${CLUSTER_ID}  flag=${FLAG}  jobs=${#ROOT_FILES[@]}  path=${DEST_BASE}/${MERGED_NAME}" \
        >> "${MANIFEST}"
    log "Manifest updated: ${MANIFEST}"
fi

log "Done. Run ${RUN_ID} stored under:"
log "  ${RUN_DIR}"