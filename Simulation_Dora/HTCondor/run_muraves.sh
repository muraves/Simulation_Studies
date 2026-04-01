#!/bin/bash

#SIF_PATH="/group/Muography/MURAVES/container/simulation_container/muraves-sim-latest.sif"
#singularity exec ${SIF_PATH} echo "Container works!"

PROC_ID=$1

SIF_PATH="/group/Muography/MURAVES/container/simulation_container/muraves-sim-latest.sif"
#SIF_PATH="/user/dgeeraer/MURAVES/muraves-sim-latest.sif"
G4DATA_HOST="/group/Muography/MURAVES/container/simulation_container/geant4_datasets"
#G4DATA_HOST="/user/dgeeraer/MURAVES/datasets"
G4DATA_CONTAINER="/root/geant4/datasets"

OUTPUT_HOST="/user/dgeeraer/MURAVES/test_output_sim"
OUTPUT_CONTAINER="/output"
#PNFS_OUTPUT="/pnfs/iihe/cms/store/user/dgeeraer/muravessim/"

singularity exec \
    --bind ${G4DATA_HOST}:${G4DATA_CONTAINER},\
${OUTPUT_HOST}:${OUTPUT_CONTAINER} \
    --pwd /user/dgeeraer/MURAVES/Simulation_Studies/Simulation_Dora/build-cont-t2b \
    --env GEANT4_DATA_DIR=${G4DATA_CONTAINER} \
    ${SIF_PATH} \
    ./MuravesSim --m run.mac

# Copy output to pnfs
#echo "Copying output to pnfs..."
#rsync -av ${OUTPUT_HOST}/* ${PNFS_OUTPUT}/ && rm -rf ${OUTPUT_HOST}/*

#echo "Done."
