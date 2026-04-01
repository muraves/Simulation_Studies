#----------------------------------------------------------------------------
# Setup the project
cmake_minimum_required(VERSION 3.16...3.27)
set(CMAKE_CXX_STANDARD 17) # this way static members can be initialized inline (e.g. in Material.hh)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
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

# CRY stuff
#
#set(CRY_PATH /home/michael/muography/simulations/CRY/cry_v1.7)
#set(CRY_PATH /Users/dorageeraerts/Documents/PhD/software/cry)
set(CRY_PATH /opt/CRY)
set(CRY_LIB -L${CRY_PATH}/lib libCRY.a) 
include_directories(${CRY_PATH}/src)

#----------------------------------------------------------------------------
# Add the executable, use our local headers, and link it to the Geant4 libraries
#
add_executable(MuravesSim MuravesSim.cc ${sources} ${headers})
target_include_directories(MuravesSim PRIVATE include ${CMAKE_CURRENT_SOURCE_DIR}/CRY)
target_link_libraries(MuravesSim PRIVATE ${CRY_LIB} ${Geant4_LIBRARIES})

#----------------------------------------------------------------------------

# For Mac arm64, ensure proper architecture flags
#if(APPLE)
    #set_target_properties(MuravesSim PROPERTIES
        #MACOSX_RPATH ON
        #ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        #RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    #)
#endif()

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
  cmd-cry-dora.file
  gps_beam_center.mac
  )

foreach(_script ${MURAVESSIM_SCRIPTS})
  configure_file(
    ${PROJECT_SOURCE_DIR}/${_script}
    ${PROJECT_BINARY_DIR}/${_script}
    COPYONLY
    )
endforeach()
