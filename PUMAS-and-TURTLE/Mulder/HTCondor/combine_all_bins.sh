#!/bin/bash
# Run manually once you've confirmed Condor has finished (e.g. via
# `condor_q <CLUSTER_ID>`). Picks up CLUSTER_ID / RUN_TAG / LOGBASE from
# run_state/<CLUSTER_ID>.env (written by submit_all_bins.sh), or from
# run_state/latest.env if no cluster id is given.
#
# 1) combines the per-job-replica files for each bin into one line per bin,
#    in a single pass over all job files (see combine_all_bins.awk)
# 2) sorts all bins into one final file, named after the actual run
#    parameters (phi range, el range, d-phi, d-el, rho, n-E, nEvents)

set -euo pipefail

source muraves_mulder_config.sh   # OUTPUT_HOST base path (and default LOGBASE, if set there)

RUN_STATE_DIR="run_state"

if [ -n "${1:-}" ]; then
    STATE_FILE="${RUN_STATE_DIR}/${1}.env"                  # ./combine_all_bins.sh 3382819 (explicit wins)
elif [ -f "${RUN_STATE_DIR}/latest.env" ]; then
    STATE_FILE="${RUN_STATE_DIR}/latest.env"                # ./combine_all_bins.sh, no args -> most recent run
elif [ -n "${CLUSTER_ID:-}" ]; then
    STATE_FILE="${RUN_STATE_DIR}/${CLUSTER_ID}.env"         # last-resort fallback if somehow no latest.env
else
    echo "ERROR: no run state available - pass a cluster id explicitly." >&2
    exit 1
fi

if [ ! -f "$STATE_FILE" ]; then
    echo "ERROR: no run-state file found ($STATE_FILE)." >&2
    echo "Pass a cluster id explicitly: ./combine_all_bins.sh <CLUSTER_ID>" >&2
    exit 1
fi

source "$STATE_FILE"   # sets CLUSTER_ID, RUN_TAG, LOGBASE (overrides config default, if any)
echo "Using CLUSTER_ID=${CLUSTER_ID} RUN_TAG=${RUN_TAG} LOGBASE=${LOGBASE} (from $STATE_FILE)" >&2

mv "${LOGBASE}/${RUN_TAG}" "${LOGBASE}/${CLUSTER_ID}" 2>/dev/null || true

if [ -f run_config.txt ]; then
    source run_config.txt   # KEY=VALUE lines written by generate_bins.sh
else
    echo "run_config.txt not found -- run ./generate_bins.sh first" >&2
    exit 1
fi

# ADJUST if your job output actually lands somewhere else: this assumes
# run_bin_job.sh copies each job's finished output to OUTPUT_HOST/CLUSTER_ID
# (its node-local scratch dir is cleaned up after copying).
INDIR="${OUTPUT_HOST}/${CLUSTER_ID}"
echo "$INDIR" >&2

if [ ! -d "$INDIR" ]; then
    echo "ERROR: $INDIR does not exist - no job output was produced (check Condor logs / mamba errors)" >&2
    exit 1
fi

PHI_MIN_ABS=$(awk -v phi="$PHI_MIN" -v off="$PHI_OFFSET" 'BEGIN {printf "%.0f", phi + off}')
PHI_MAX_ABS=$(awk -v phi="$PHI_MAX" -v off="$PHI_OFFSET" 'BEGIN {printf "%.0f", phi + off}')

FINAL="${INDIR}/flux_all_bins_phi${PHI_MIN_ABS}-${PHI_MAX_ABS}_el${EL_MIN}-${EL_MAX}_dphi${DPHI}_del${DEL}_rho${RHO}_nE${N_E}_nEvents${NEVENTS}.txt"

UNSORTED="${INDIR}/.flux_all_bins_unsorted.txt"
EXPECTED_BINS_FILE="${INDIR}/.expected_bins.txt"
STDERR_LOG="${INDIR}/.combine_stderr.log"
trap 'rm -f "$UNSORTED" "$EXPECTED_BINS_FILE" "$STDERR_LOG"' EXIT

# Build the expected-bins list (az_abs el), same transform the old
# per-bin loop applied before calling combine_bin.sh.
awk -v off="$PHI_OFFSET" '
    NF == 0 { next }
    { printf "%.6f %s\n", $1 + off, $2 }
' bins.txt > "$EXPECTED_BINS_FILE"

# Gather all job-replica files once. Adjust the glob if your job output
# naming ever changes -- must match what run_bin_job.sh actually writes.
shopt -s nullglob
DATA_FILES=( "${INDIR}"/flux_5m_*bin_rho*_discrete_100T_az*_job*.txt )
shopt -u nullglob

if [ ${#DATA_FILES[@]} -eq 0 ]; then
    echo "ERROR: no job output files found in $INDIR" >&2
    exit 1
fi

# Single pass over every file, bucketed by bin, instead of one pass per bin.
awk -v dphi="$DPHI" -f combine_all_bins.awk "$EXPECTED_BINS_FILE" "${DATA_FILES[@]}" \
    > "$UNSORTED" 2> "$STDERR_LOG"

# Surface the per-bin warnings exactly like before, and pull out the count.
grep -v '^MISSING_COUNT=' "$STDERR_LOG" >&2 || true
MISSING=$(grep '^MISSING_COUNT=' "$STDERR_LOG" | tail -1 | cut -d= -f2)
MISSING=${MISSING:-0}

sort -k1,1n -k2,2n "$UNSORTED" > "$FINAL"

EXPECTED_BINS=$(wc -l < bins.txt)
ACTUAL_BINS=$(wc -l < "$FINAL")

echo "Final combined file: $FINAL ($ACTUAL_BINS / $EXPECTED_BINS bins)"

if [ ! -s "$FINAL" ] || [ "$MISSING" -gt 0 ] || [ "$ACTUAL_BINS" -ne "$EXPECTED_BINS" ]; then
    echo "WARNING: run incomplete ($MISSING bin(s) missing) - leaving intermediate files in $INDIR for debugging, NOT cleaning up" >&2
    exit 1
fi

echo "All $EXPECTED_BINS bins present - cleaning up intermediate files, keeping only $FINAL"
find "$INDIR" -maxdepth 1 -name 'flux_5m_*_job*.txt' -delete