#
#
# (C) Copyright 2009-2011 Johns Hopkins University (JHU), All Rights Reserved.
#
# --- begin cisst license - do not edit ---
#
# This software is provided "as is" under an open source license, with
# no warranty.  The complete license can be found in license.txt and
# http://www.cisst.org/cisst/license.txt.
#
# --- end cisst license ---

set (REQUIRED_CISST_LIBRARIES cisstCommon
                              cisstVector
                              cisstOSAbstraction
                              cisstMultiTask
                              cisstStereoVision
                              cisstStereoVisionQt)

# find cisst with all available libraries
find_package (cisst REQUIRED ${REQUIRED_CISST_LIBRARIES} QUIET)

if (cisst_FOUND)

  # load cisst configuration
  include (${CISST_USE_FILE})

  cisst_find_saw_component (sawOpenNI REQUIRED)

  if (sawOpenNI_FOUND)

    include_directories (${sawOpenNI_INCLUDE_DIR})
    link_directories (${sawOpenNI_LIBRARY_DIR})

    # Copy OpenNI Kinect XML config file to the build tree
    if (XCODE_VERSION OR MSVC)
      # Debug
      configure_file (${CMAKE_CURRENT_SOURCE_DIR}/SamplesConfig.xml
                      ${EXECUTABLE_OUTPUT_PATH}/Debug/SamplesConfig.xml COPYONLY)
      # Release
      configure_file (${CMAKE_CURRENT_SOURCE_DIR}/SamplesConfig.xml
                      ${EXECUTABLE_OUTPUT_PATH}/Release/SamplesConfig.xml COPYONLY)
      if (MSVC)
        # Release with Debug Info
        configure_file (${CMAKE_CURRENT_SOURCE_DIR}/SamplesConfig.xml
                        ${EXECUTABLE_OUTPUT_PATH}/RelWithDebInfo/SamplesConfig.xml COPYONLY)
        # Default MSVC debug directory (when launching program from within MSVC)
        configure_file (${CMAKE_CURRENT_SOURCE_DIR}/SamplesConfig.xml
                        ${CMAKE_CURRENT_BINARY_DIR}/SamplesConfig.xml COPYONLY)
      endif (MSVC)
    else (XCODE_VERSION OR MSVC)
      configure_file (${CMAKE_CURRENT_SOURCE_DIR}/SamplesConfig.xml
                      ${EXECUTABLE_OUTPUT_PATH}/SamplesConfig.xml COPYONLY)
    endif (XCODE_VERSION OR MSVC)

    # example without cisstStereoVision
    add_executable (oniExKinectTest oniExKinectTest.cpp)
    target_link_libraries (oniExKinectTest sawOpenNI ${OPENNI_LIBRARIES})
    cisst_target_link_libraries (oniExKinectTest cisstCommon cisstVector cisstOSAbstraction)

    # copy the configration file to binary tree
    add_custom_command (TARGET oniExKinectTest
                        POST_BUILD
                        COMMAND ${CMAKE_COMMAND}
                        ARGS -E copy
                             ${CMAKE_CURRENT_SOURCE_DIR}/SamplesConfig.xml
                             ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/SamplesConfig.xml
                        COMMENT "Copy Kinect configuration file to binary tree")

    # examples with cisstStereoVision
    if (sawOpenNI_HAS_SVL)
        # svl example 1
        add_executable (oniExKinectView oniExKinectView.cpp)
        target_link_libraries (oniExKinectView sawOpenNI ${OPENNI_LIBRARIES})
        cisst_target_link_libraries (oniExKinectView ${REQUIRED_CISST_LIBRARIES})

        # svl example 2
        add_executable (oniExKinectPlaneFinder
                        oniExKinectPlaneFinder.cpp)
        target_link_libraries (oniExKinectPlaneFinder sawOpenNI ${OPENNI_LIBRARIES})
        cisst_target_link_libraries (oniExKinectPlaneFinder ${REQUIRED_CISST_LIBRARIES})
    else (sawOpenNI_HAS_SVL)
      message ("Information: examples oniExKinectView and oniExKinectPlaneFinder in ${CMAKE_CURRENT_SOURCE_DIR} will not be compiled, it requires cisstStereoVision")
    endif (sawOpenNI_HAS_SVL)

  endif (sawOpenNI_FOUND)

endif (cisst_FOUND)

