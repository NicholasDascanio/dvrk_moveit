#
#
# (C) Copyright 2011-2012 Johns Hopkins University (JHU), All Rights
# Reserved.
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
                              cisstStereoVision)

find_package (cisst REQUIRED ${REQUIRED_CISST_LIBRARIES} QUIET)

if (cisst_FOUND)

  # load cisst configuration
  include (${CISST_USE_FILE})

  set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}")
  find_package (OpenNI REQUIRED)

  if (OPENNI_FOUND)

    set (sawOpenNI_INCLUDE_DIR "${sawOpenNI_SOURCE_DIR}/include")
    set (sawOpenNI_LIBRARY_DIR "${LIBRARY_OUTPUT_PATH}")
    set (sawOpenNI_LIBRARIES sawOpenNI ${OPENNI_LIBRARIES})

    include_directories (${OPENNI_INCLUDE_DIRS} ${sawOpenNI_INCLUDE_DIR})
    set (sawOpenNI_HEADER_DIR "${sawOpenNI_SOURCE_DIR}/include/sawOpenNI")
    set (HEADER_FILES
         ${sawOpenNI_HEADER_DIR}/sawOpenNIExport.h
         ${sawOpenNI_HEADER_DIR}/osaOpenNI.h
         ${sawOpenNI_HEADER_DIR}/osaOpenNISkeleton.h
         osaOpenNIData.h
         )
    set (SOURCE_FILES
         osaOpenNI.cpp
         osaOpenNISkeleton.cpp
         osaOpenNIData.cpp
         )

    if (CISST_cisstStereoVision)
      set (HEADER_FILES
           ${HEADER_FILES}
           ${sawOpenNI_HEADER_DIR}/oniPlaneSegmentation.h
           ${sawOpenNI_HEADER_DIR}/svlFilterSource3DSensorFile.h
           ${sawOpenNI_HEADER_DIR}/svlFilterSourceKinect.h
           ${sawOpenNI_HEADER_DIR}/svlFilterPlaneSegmentation.h
           )
      set (SOURCE_FILES
           ${SOURCE_FILES}
           planefit.h
           oniPlaneSegmentation.cpp
           svlFilterSource3DSensorFile.cpp
           svlFilterSourceKinect.cpp
           svlFilterPlaneSegmentation.cpp
           )
    else (CISST_cisstStereoVision)
      message ("Information: some files in ${CMAKE_CURRENT_SOURCE_DIR} will not be compiled as they require cisstStereoVision")
    endif (CISST_cisstStereoVision)


    add_library (sawOpenNI
                 ${HEADER_FILES}
                 ${SOURCE_FILES}
                 )

    target_link_libraries (sawOpenNI
                           ${OPENNI_LIBRARIES}
                           )

    if (CISST_cisstStereoVision)
      cisst_target_link_libraries (sawOpenNI ${REQUIRED_CISST_LIBRARIES})
    else (CISST_cisstStereoVision)
      cisst_target_link_libraries (sawOpenNI cisstCommon cisstVector cisstOSAbstraction)
    endif (CISST_cisstStereoVision)

    configure_file ("${CMAKE_CURRENT_SOURCE_DIR}/sawOpenNIConfig.cmake.in"
                    "${sawOpenNI_BINARY_DIR}/sawOpenNIConfig.cmake"
                    @ONLY@
                    )

    # Install targets for headers and library
    install (FILES ${HEADER_FILES}
             DESTINATION include/sawOpenNI)

    install (TARGETS sawOpenNI
             RUNTIME DESTINATION bin
             LIBRARY DESTINATION lib
             ARCHIVE DESTINATION lib)

    install (FILES "${sawOpenNI_BINARY_DIR}/sawOpenNIConfig.cmake"
             DESTINATION "${CISST_CMAKE_INSTALL_SUFFIX}/saw")

    install (FILES "${sawOpenNI_SOURCE_DIR}/code/FindOpenNI.cmake"
             DESTINATION ${CISST_CMAKE_MODULES_INSTALL_SUFFIX})

  else (OPENNI_FOUND)
    message (SEND_ERROR "code in ${CMAKE_CURRENT_SOURCE_DIR} requires OpenNI.")
  endif (OPENNI_FOUND)

else (cisst_FOUND)
  message ("Information: code in ${CMAKE_CURRENT_SOURCE_DIR} will not be compiled, it requires ${REQUIRED_CISST_LIBRARIES}")
endif (cisst_FOUND)
