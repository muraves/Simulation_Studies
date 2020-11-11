# MuravesSim
#
# Michael Tytgat
#

prerequisites:
* Geant4
* CRY
* ROOT (v6.18.04 works for me)

Modify CMakeLists.txt to specify CRY_PATH according to your local setup

To compile:
mkdir build
cd build
ccmake ..
press "configure", "generate and exit"
make

To run:
./MuravesSim --generator CRY/PartGun/GPS --inputfile <macro>

Output:
MuravesSim.root




