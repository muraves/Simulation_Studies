# Binary file to ASCII converter 
### Amrutha Samalan

prerequisites:
* COSRIKA
* FORTRAN compiler (eg: gfortran)
* Inside the CORSIKA installed folder, in the subdirectory /src/utils, FORTRAN routines for different tasks are available. Run any of the compilation link  listed (gfortran -O0 -fbounds-check readcsk2asci.f -o readcsk2asci) inside the file readcsk2asci.f. This will create an executable "readcsk2asci" inside the folder (Refer slides for images).


Steps for converting CORSIKA generated binary files to ASCII files

1. Generate datafiles in binary format
2. Keep all these files in a single folder. An example folder is uploaded (binary_files_flujo_central_europe-60sec).
3. Add the path of the CORSIKA's utils folder in you bash script (example: export PATH=/home/amrutha/CORSIKA/dpmjet/src/utils:$PATH). This is necessary for running the executable file ('readcsk2asci') from any location.
4. Download the python script (configfile.py)
5. In the Python code, specify the parameters required for the conversion <br/>
      line 12: Name of the config file you want to create (example name is given)<br/>
      line 14: Total no of showers generated or required to convert<br/>
      line 16: No of showers of each binary file which required to be converted (an example is given, preferably a constant number. Otherwise, code need be changed 
               accordingly)<br/>
      line 18: Path to the folder which contains the binary file  
               
6. Run the code 

Result:  A folder named "g4_ascii" will be created in the same directory in which you can find all the binary files converted to the ASCII format
      
