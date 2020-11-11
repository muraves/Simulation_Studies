# MuravesSim
### Michael Tytgat

prerequisites:
* Geant4
* CRY
* ROOT (v6.18.04 works for me)

Modify CMakeLists.txt to specify CRY_PATH according to your local setup

To compile:<br>
*mkdir build* <br>
*cd build* <br>
*ccmake ..* <br>
press "configure", "generate and exit" <br>
*make*

To run: <br>
*./MuravesSim --generator CRY/PartGun/GPS --inputfile \<macro\>*

Output: <br>
MuravesSim.root




