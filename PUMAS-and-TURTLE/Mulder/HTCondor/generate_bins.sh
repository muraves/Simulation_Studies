#!/bin/bash
# generate_bins.sh [PHI_MIN] [PHI_MAX] [EL_MIN] [EL_MAX] [DPHI] [DEL] [RHO] [N_E] [NEVENTS] [N_AZ_PER_JOB]
#
# All parameters default to muraves_mulder_config.sh, the single source of
# truth for a real run. Positional args are OPTIONAL and only meant for
# quick one-off test grids (e.g. a tiny 2x2 grid to sanity-check chunking)
# without editing the config file. For a real run, call with no arguments
# at all so nothing can silently diverge from the config.
#
# Writes:
#   run_config.txt - all run parameters actually used, reused by every
#                    downstream script (this is what everything else trusts,
#                    NOT muraves_mulder_config.sh directly - so a test run's
#                    overrides can't leak into a later real run by accident)
#   bins.txt       - one row per individual az/el bin (used by combine_all_bins.sh)
#   chunks.txt     - one row per az-chunk per el: "az_start az_end el"
#                    (used by generate_job_list.sh to build Condor jobs)
set -e

source muraves_mulder_config.sh

PHI_MIN=${1:-$PHI_MIN}
PHI_MAX=${2:-$PHI_MAX}
EL_MIN=${3:-$EL_MIN}
EL_MAX=${4:-$EL_MAX}
DPHI=${5:-$DPHI}
DEL=${6:-$DEL}
RHO=${7:-$RHO}
N_E=${8:-$N_E}
NEVENTS=${9:-$NEVENTS}
N_AZ_PER_JOB=${10:-$N_AZ_PER_JOB}

cat > run_config.txt <<CFG
PHI_MIN=$PHI_MIN
PHI_MAX=$PHI_MAX
EL_MIN=$EL_MIN
EL_MAX=$EL_MAX
DPHI=$DPHI
DEL=$DEL
RHO=$RHO
N_E=$N_E
NEVENTS=$NEVENTS
N_AZ_PER_JOB=$N_AZ_PER_JOB
CFG

> bins.txt
> chunks.txt

awk -v phi_min="$PHI_MIN" -v phi_max="$PHI_MAX" -v el_min="$EL_MIN" -v el_max="$EL_MAX" \
    -v dphi="$DPHI" -v del="$DEL" -v n_az="$N_AZ_PER_JOB" '
BEGIN {
    for (el = el_min; el < el_max - 1e-9; el += del) {
        chunk_start = phi_min
        count = 0
        for (az = phi_min; az < phi_max - 1e-9; az += dphi) {
            printf "%.6g %.6g\n", az, el >> "bins.txt"
            count++
            if (count == n_az) {
                az_end = az + dphi
                printf "%.6g %.6g %.6g\n", chunk_start, az_end, el >> "chunks.txt"
                chunk_start = az_end
                count = 0
            }
        }
        # flush a final partial chunk (fewer than n_az az bins left over)
        if (count > 0) {
            az_end = chunk_start + count * dphi
            printf "%.6g %.6g %.6g\n", chunk_start, az_end, el >> "chunks.txt"
        }
    }
}'

echo "Wrote $(wc -l < bins.txt) bins and $(wc -l < chunks.txt) chunks to bins.txt / chunks.txt"