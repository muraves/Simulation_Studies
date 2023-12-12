### Thickness_TURTLE_025_30.txt:
Thickness of Mt. Vesuvius and surrounding area at the observation point of the MURAVES experiment, as function of azimuth and elevation angle. \
The loop of each block of data start Elevation 0-30 degree and from Azmuth 0-360 degree, there are 4 bins in each angle. \
The starting point for Azimuth agle is Absolute North.

### FluxPUMAS025_005.txt
each data set consists of Azimuth, elevation angle, flux simulated with PUMAS Backward mode and error. \
Standard rock density is set to 2.65E+03 kg/m3. The final energy range fed into PUMAS Backward mode is 0.005 to 3000 GeV. \


## A How to Approach
### In TURTLE:
1. use: TURTLE/example-projection.c to run the DEM file and location coordinate of the MURAVES in the DEM file. 
   get: 2D overview prjected map of DEM, and Geodetic Coordinate of the MURAVES experiment.
2. use: TURTLE/example-stepper.c with the projected map and MURAVES coordinate.
   get: Rockthickness map of the Mt. Vesuvius at the observation point MURAVES, in Azimuth and Elevation angle.

### In PUMAS:
use: PUMAS/geomentry.c with the Rockthickness map file.
define: medium rock+air; backward mode; finally energy range; energy loss mode and so on.
get: flux in Azimuth and Elevation angle. 

