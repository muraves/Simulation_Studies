#!/bin/bash
# generate_job_list.sh [N_JOBS_PER_BIN]
# Defaults to N_JOBS_PER_BIN from muraves_mulder_config.sh - pass an
# argument only to override for a one-off test run.
#
# Expands chunks.txt x N_JOBS_PER_BIN replicas -> all_jobs.txt (az_start az_end el job)
# all_jobs.txt is used as Condor itemdata: "queue az_start,az_end,el,job from all_jobs.txt"
set -e

source muraves_mulder_config.sh
N_JOBS_PER_BIN=${1:-$N_JOBS_PER_BIN}

if [ ! -f chunks.txt ]; then
    echo "chunks.txt not found - run generate_bins.sh first" >&2
    exit 1
fi

> all_jobs.txt
while read -r AZ_START AZ_END EL; do
    for ((j=1; j<=N_JOBS_PER_BIN; j++)); do
        echo "$AZ_START $AZ_END $EL $j" >> all_jobs.txt
    done
done < chunks.txt

echo "Wrote $(wc -l < all_jobs.txt) jobs to all_jobs.txt"