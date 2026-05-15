This Geant4 application simulated the cosmic muon flux passing through the MURAVES detector. This includes: the muraves detector geometry, a modeled ground, integrated EcoMug cosmic muon generator (+ other generators —CRY, Particle Gun, General Particle Source— that can be used for testing or other purposes, have to be configured though based on purpose), sensitive detector to acquire particle hits, output saving in a root file, text file with metadata of a run, configure files for the EcoMug generator and detector configuration.

# User Guide (documentation under construction!) 

# A. Local computer with Geant4 installed
This Geant4 application works for Geant4 version 11.2.2 and higher. 
The cosmic-ray generator CRY (version 1.7) has to be installed: https://nuclear.llnl.gov/simulation/ 

To build the Geant4 application:
1. Download or clone the Geant4-simulation directory
2. Create a new build directory
3. In the build directory, run the following command:
```bash
 cmake -DCRY_PATH=<path/to/cry/> ..
```
4. Then run the command
```bash
make
```
5. to build the Geant4 application

To run the Geant4 application in interactive mode:
1. Run
```bash
 ./MuravesSim <macrofile>
```
! Make sure the gui.mac and init_vis.mac are inside the build directory, you also have to install a visualisation driver (https://geant4.web.cern.ch/documentation/dev/bfad_html/ForApplicationDevelopers/Visualization/introduction.html). 

To run the Geant4 application in batch mode:
1. Run
```bash
 ./MuravesSim <macrofile>
```
Usually this macrofile is called run.mac and is used to configure the simulation (i.e., detector geometry, particle generation, output paths)
(example of run.mac with explanation: to be added here)

The command-line arguments are: 
--m: to give a macro file as input
--ui: to start up a user interface (e.g. if you want to pass a macro file just for testing the detector configuration (so not directly running a lot of events), and want to check this in visualisation, you would start up ui)
--threads: to specify the number of threads (both sequential mode and multi-threading are supported)

# B. On T2B cluster

The steps of building and running the application are the same as in A. However, in this case, the simulation has to be build (using CRY_PATH=/opt/CRY) and run in a container with Geant4 and other dependencies (e.g., CRY) installed. 
An image of this container can be found in /group/Muography/container/simulation_container/muraves-sim-latest.sif
In the container, no visualisation is possible, however, it is still possible to use the application in interactive mode. 
A container can be opened by running: 
```bash
singularity shell -B <output/directory/in/user>:/output /group/Muography/MURAVES/container/simulation_container/muraves-sim-latest.sif
```
The datasets have to be set before running the simulation, by running the following command inside the container: 
```bash
export GEANT4_DATA_DIR=/user/dgeeraer/MURAVES/datasets
```
