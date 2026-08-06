#!/bin/bash
# submit_all_bins.sh [N_JOBS_PER_BIN] [MAX_IDLE]
#
# Both default to muraves_mulder_config.sh - pass arguments only to override
# for a one-off run.
#
# Builds all_jobs.txt, fills in the Condor submit template, submits it,
# parses the cluster ID straight out of condor_submit's own output, and
# saves CLUSTER_ID/RUN_TAG/LOGBASE to run_state/ so combine_all_bins.sh
# can pick them up automatically once you check the jobs have finished
# (e.g. via condor_q) and run it manually.
set -e

source muraves_mulder_config.sh
N_JOBS_PER_BIN=${1:-$N_JOBS_PER_BIN}
MAX_IDLE=${2:-$MAX_IDLE}

./generate_bins.sh

if [ ! -f run_config.txt ]; then
    echo "run_config.txt not found - run generate_bins.sh first" >&2
    exit 1
fi
source run_config.txt   # DPHI DEL RHO N_E NEVENTS ...

./generate_job_list.sh "$N_JOBS_PER_BIN"

LOGBASE="/user/dgeeraer/MURAVES/output_mulder/output_htcondor"
#RUN_TAG="staging_$$_$(date +%s)"
RUN_TAG="staging_$(date +%Y%m%d_%H%M%S)"

mkdir -p "${LOGBASE}/${RUN_TAG}"

sed -e "s|__DPHI__|${DPHI}|g" \
    -e "s|__DEL__|${DEL}|g" \
    -e "s|__RHO__|${RHO}|g" \
    -e "s|__N_E__|${N_E}|g" \
    -e "s|__NEVENTS__|${NEVENTS}|g" \
    -e "s|__MAX_IDLE__|${MAX_IDLE}|g" \
    -e "s|__RUN_TAG__|${RUN_TAG}|g" \
    mulder.sub > generated_submit.condor

SUBMIT_OUT=$(condor_submit generated_submit.condor)
echo "$SUBMIT_OUT"

# condor_submit prints e.g. "N job(s) submitted to cluster 1234."
CLUSTER_ID=$(echo "$SUBMIT_OUT" | sed -n 's/.* submitted to cluster \([0-9][0-9]*\)\..*/\1/p')
if [ -z "$CLUSTER_ID" ]; then
    echo "ERROR: could not parse cluster ID from condor_submit output" >&2
    exit 1
fi
echo "Cluster ID: $CLUSTER_ID"

RUN_STATE_DIR="run_state"
mkdir -p "$RUN_STATE_DIR"

cat > "${RUN_STATE_DIR}/${CLUSTER_ID}.env" <<EOF
CLUSTER_ID=${CLUSTER_ID}
RUN_TAG=${RUN_TAG}
LOGBASE=${LOGBASE}
EOF
ln -sf "${CLUSTER_ID}.env" "${RUN_STATE_DIR}/latest.env"

echo "Submitted cluster ${CLUSTER_ID} (tag ${RUN_TAG})"
echo "When jobs finish (check with condor_q ${CLUSTER_ID}), just run: ./combine_all_bins.sh"