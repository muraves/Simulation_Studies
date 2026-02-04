#----------------------------------------------------------------------------
# Setup the project
cmake_minimum_required(VERSION 3.16...3.27)
project(MuravesSim)

#----------------------------------------------------------------------------
# Find Geant4 package, activating all available UI and Vis drivers by default
# See the documentation for a guide on how to enable/disable specific components
#
find_package(Geant4 REQUIRED ui_all vis_all)

#----------------------------------------------------------------------------
# Locate sources and headers for this project
# NB: headers are included so they will show up in IDEs
#
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)

#----------------------------------------------------------------------------
# Add the executable, use our local headers, and link it to the Geant4 libraries
#
add_executable(MuravesSim MuravesSim.cc ${sources} ${headers})
target_include_directories(MuravesSim PRIVATE include)
target_link_libraries(MuravesSim PRIVATE ${Geant4_LIBRARIES})

#----------------------------------------------------------------------------
#
# CRY stuff
#
#set(CRY_PATH /home/michael/muography/simulations/CRY/cry_v1.7)
#set(CRY_LIB -L${CRY_PATH}/lib libCRY.a) 
#include_directories(${CRY_PATH}/src)

#----------------------------------------------------------------------------
# Copy all scripts to the build directory, i.e. the directory in which we
# build B5. This is so that we can run the executable directly because it
# relies on these scripts being in the current working directory.
#
set(MURAVESSIM_SCRIPTS
  MuravesSim.in
  MuravesSim.out
  icons.mac
  gui.mac
  run.png
  init_vis.mac
  run1.mac
  run2.mac
  vis.mac
  plotter.mac
  tsg_offscreen.mac
  cmd.file
  cmd2.file
  cmdpartgun.mac
  cmdgps.mac
  )

foreach(_script ${MURAVESSIM_SCRIPTS})
  configure_file(
    ${PROJECT_SOURCE_DIR}/${_script}
    ${PROJECT_BINARY_DIR}/${_script}
    COPYONLY
    )
endforeach()
