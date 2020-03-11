#
#
# (C) Copyright 2011-2012 Johns Hopkins University (JHU), All Rights Reserved.
#
# --- begin cisst license - do not edit ---
#
# This software is provided "as is" under an open source license, with
# no warranty.  The complete license can be found in license.txt and
# http://www.cisst.org/cisst/license.txt.
#
# --- end cisst license ---

set (REQUIRED_CISST_LIBRARIES
     cisstCommon
     cisstVector
     cisstNumerical
     cisstOSAbstraction
     cisstRobot
     cisstMultiTask
     cisstParameterTypes
     )

find_package (cisst REQUIRED ${REQUIRED_CISST_LIBRARIES})

find_package (OpenGL REQUIRED)
find_package (GLUT REQUIRED)

if(cisst_FOUND_AS_REQUIRED AND OPENGL_FOUND AND GLUT_FOUND)

  # load cisst configuration
  include (${CISST_USE_FILE})

  # create/configure file for find_package (sawGLUTSimulator)
  set (sawGLUTSimulator_INCLUDE_DIR "${sawGLUTSimulator_SOURCE_DIR}/include")
  set (sawGLUTSimulator_LIBRARY_DIR "${LIBRARY_OUTPUT_PATH}")
  set (sawGLUTSimulator_LIBRARIES sawGLUTSimulator)

  include_directories(${OPENGL_INCLUDE_DIR} ${GLUT_INCLUDE_DIR} ${sawGLUTSimulator_INCLUDE_DIR})
  set (sawGLUTSimulator_HEADER_DIR "${sawGLUTSimulator_SOURCE_DIR}/include/sawGLUTSimulator")

  set(HEADER_FILES
    osaGeometry.h
    osaMeshTriangular.h
    ${sawGLUTSimulator_HEADER_DIR}/osaGLUT.h
    ${sawGLUTSimulator_HEADER_DIR}/osaGLUTManipulator.h
    ${sawGLUTSimulator_HEADER_DIR}/mtsGLUTManipulator.h
    ${sawGLUTSimulator_HEADER_DIR}/sawGLUTSimulatorExport.h
    )

  set(SOURCE_FILES
    osaGeometry.cpp
    osaMeshTriangular.cpp
    osaGLUT.cpp
    osaGLUTManipulator.cpp
    mtsGLUTManipulator.cpp
    )

  add_library(sawGLUTSimulator ${HEADER_FILES} ${SOURCE_FILES})
  set_property (TARGET sawGLUTSimulator PROPERTY FOLDER "sawGLUTSimulator")
  target_link_libraries(sawGLUTSimulator ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})
  cisst_target_link_libraries(sawGLUTSimulator ${REQUIRED_CISST_LIBRARIES})

  configure_file ("${CMAKE_CURRENT_SOURCE_DIR}/sawGLUTSimulatorConfig.cmake.in"
                  "${sawGLUTSimulator_BINARY_DIR}/sawGLUTSimulatorConfig.cmake"
                   @ONLY@)

  # Install target for headers and library
  install (DIRECTORY
           ${sawGLUTSimulator_INCLUDE_DIR}/sawGLUTSimulator
           DESTINATION include
           PATTERN .svn EXCLUDE)

  install (TARGETS sawGLUTSimulator
           RUNTIME DESTINATION bin
           LIBRARY DESTINATION lib
           ARCHIVE DESTINATION lib)

  install (FILES "${sawGLUTSimulator_BINARY_DIR}/sawGLUTSimulatorConfig.cmake"
           DESTINATION "${CISST_CMAKE_INSTALL_SUFFIX}/saw")

else(cisst_FOUND_AS_REQUIRED AND OPENGL_FOUND AND GLUT_FOUND)
  message(WARNING "sawGLUTSimulator will not be built (missing dependencies)")
endif(cisst_FOUND_AS_REQUIRED AND OPENGL_FOUND AND GLUT_FOUND)
