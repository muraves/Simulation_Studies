#!/bin/bash
# muraves_mulder_config.sh
# Shared paths, container settings, and default parameters.
# Sourced by: run_all.sh, run_bin_job.sh, run_local_test.sh,
#             submit_all_bins.sh, combine_all_bins.sh
#
# OUTPUT_HOST is the static base directory - each run's actual output lives
# under OUTPUT_HOST/$CLUSTER_ID, built by run_bin_job.sh / combine_all_bins.sh
# once CLUSTER_ID is known (Condor's own $(Cluster) macro, filled in by
# Condor itself at submit time for job-side scripts, and parsed out of
# `condor_submit`'s output for combine_all_bins.sh - see submit_all_bins.sh).

# --- container / paths ---
SIF_PATH="/user/dgeeraer/MURAVES/mulder-container.sif"
DATA_PATH_HOST="/user/dgeeraer/MURAVES/Simulation_Studies/PUMAS-and-TURTLE/Mulder/"
DATA_PATH_CONTAINER="/data/"
OUTPUT_HOST="/user/dgeeraer/MURAVES/output_mulder"
OUTPUT_CONTAINER="/output"

# --- physics defaults ---
PHI_OFFSET=136.0 # offset of 136 because phi is w.r.t. absolute North in Mulder
PHI_MIN=$(awk -v o="$PHI_OFFSET" 'BEGIN {print 150.0 - o}')
PHI_MAX=$(awk -v o="$PHI_OFFSET" 'BEGIN {print 210.0 - o}')
EL_MIN=0.
EL_MAX=40.
DPHI=0.2
DEL=0.2
RHO=2.65E3
N_E=10000
NEVENTS=100

# --- chunking / Condor throttling ---
N_AZ_PER_JOB=5   # az bins grouped into one Condor job (per el)
N_JOBS_PER_BIN=1    # MC replicas per chunk
MAX_IDLE=1000        # Condor max_idle across the whole cluster