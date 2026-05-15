#!/bin/bash

#SIF_PATH="/group/Muography/MURAVES/container/simulation_container/muraves-sim-latest.sif"
#singularity exec ${SIF_PATH} echo "Container works!"

PROC_ID=$1
echo "$PROC_ID"

echo "Hello from worker node!"
hostname
date