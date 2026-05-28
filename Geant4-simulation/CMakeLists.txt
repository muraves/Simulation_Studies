#----------------------------------------------------------------------------
# Setup the project
cmake_minimum_required(VERSION 3.16...3.27)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(MuravesSim)

#----------------------------------------------------------------------------
# Find Geant4 package, activating all available UI and Vis drivers by default
#
find_package(Geant4 REQUIRED ui_all vis_all)

#----------------------------------------------------------------------------
# Locate sources and headers for this project
#
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)

#----------------------------------------------------------------------------
# CRY stuff
#set(CRY_PATH /Users/dorageeraerts/Documents/PhD/software/cry)
#set(CRY_PATH /opt/CRY)

# in terminal do: cmake -DCRY_PATH=/Users/dorageeraerts/Documents/PhD/software/cry ..
#
set(CRY_PATH "" CACHE PATH "Path to CRY installation")
if(NOT CRY_PATH)
    message(FATAL_ERROR "CRY_PATH not set! Pass -DCRY_PATH=/path/to/cry to cmake.")
endif()

set(CRY_LIB -L${CRY_PATH}/lib libCRY.a)
include_directories(${CRY_PATH}/src)

#----------------------------------------------------------------------------
# Get Git version info and generate header
#
find_package(Git QUIET)
if(GIT_FOUND)
  execute_process(
    COMMAND ${GIT_EXECUTABLE} describe --tags --always --dirty
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_DESCRIBE
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
  )
  execute_process(
    COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
  )
  execute_process(
    COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
  )
else()
  set(GIT_DESCRIBE "unknown")
  set(GIT_COMMIT_HASH "unknown")
  set(GIT_BRANCH "unknown")
endif()

configure_file(
  ${CMAKE_SOURCE_DIR}/include/GitVersion.hh.in
  ${CMAKE_BINARY_DIR}/include/GitVersion.hh
)

#----------------------------------------------------------------------------
# Add the executable and link to libraries
#
add_executable(MuravesSim MuravesSim.cc ${sources} ${headers})
target_include_directories(MuravesSim PRIVATE
    include
    ${CMAKE_CURRENT_SOURCE_DIR}/CRY
    ${CMAKE_BINARY_DIR}/include
)
target_link_libraries(MuravesSim PRIVATE ${CRY_LIB} ${Geant4_LIBRARIES})

#----------------------------------------------------------------------------
# Copy scripts to the build directory
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