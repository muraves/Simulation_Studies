#!/bin/bash
# Runs the whole NPE conversion pipeline in order, using config.cfg.
# Stops on the first failure, since each stage depends on the previous one's output.
set -e

cd "$(dirname "$0")"

root -l -b -q '1_select_4p_tracks.C'
root -l -b -q '2_landau_fit.C'
root -l -b -q '3_merge_sim_add_uid.C'
root -l -b -q '4_add_energy_npe.C'
