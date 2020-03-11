#
#
# (C) Copyright 2012 Johns Hopkins University (JHU), All Rights
# Reserved.
#
# --- begin cisst license - do not edit ---
#
# This software is provided "as is" under an open source license, with
# no warranty.  The complete license can be found in license.txt and
# http://www.cisst.org/cisst/license.txt.
#
# --- end cisst license ---

set(
  REQUIRED_CISST_LIBRARIES
  cisstCommon
  cisstVector
  cisstNumerical
  cisstOSAbstraction
  cisstMultiTask
  cisstParameterTypes
  cisstRobot )

find_package( cisst REQUIRED ${REQUIRED_CISST_LIBRARIES} )

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")
find_package( OpenDynamicsEngine )

if( cisst_FOUND_AS_REQUIRED AND ODE_FOUND )

  include( ${CISST_USE_FILE} )
  cisst_find_saw_component( sawOpenSceneGraph REQUIRED  )

  if( sawOpenSceneGraph_FOUND )

    # create/configure file for find_package (sawKeyboard)
    set(sawOpenDynamicsEngine_LIBRARY_DIR ${LIBRARY_OUTPUT_PATH} ${sawOpenSceneGraph_LIBRARY_DIR})
    set(sawOpenDynamicsEngine_LIBRARIES sawOpenDynamicsEngine ${sawOpenSceneGraph_LIBRARIES} )
    set(sawOpenDynamicsEngine_DEFINITIONS "-DdDOUBLE" )
    set(sawOpenDynamicsEngine_INCLUDE_DIRS
        ${sawOpenSceneGraph_INCLUDE_DIRS}
        "${sawOpenDynamicsEngine_SOURCE_DIR}/include")

    include_directories(
      ${sawOpenDynamicsEngine_INCLUDE_DIRS}
      ${OpenDynamicsEngine_INCLUDE_DIRS} )

    set (HEADER_FILE_NAMES
      osaODEBarrettHand.h
      osaODEBody.h
      osaODEJoint.h
      osaODEManipulator.h
      osaODEServoMotor.h
      osaODEWorld.h

      mtsODEManipulatorTask.h
      mtsODEWorld.h

      mtsODEManipulator.h
      mtsODEBarrettHand.h

      sawOpenDynamicsEngineExport.h
      )

    set ( SOURCE_FILES
      osaODEBarrettHand.cpp
      osaODEBody.cpp
      osaODEJoint.cpp
      osaODEManipulator.cpp
      osaODEServoMotor.cpp
      osaODEWorld.cpp

      mtsODEManipulatorTask.cpp
      )

    unset (HEADER_FILES)
    foreach (_header ${HEADER_FILE_NAMES})
      set (HEADER_FILES ${HEADER_FILES} "${sawOpenDynamicsEngine_SOURCE_DIR}/include/sawOpenDynamicsEngine/${_header}")
    endforeach ()

    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCISST_ODE_SUPPORT -DdDOUBLE" )

    add_library( sawOpenDynamicsEngine ${SOURCE_FILES} ${HEADER_FILES})
    set_property (TARGET sawOpenDynamicsEngine PROPERTY FOLDER "sawOpenDynamicsEngine")

    target_link_libraries(
      sawOpenDynamicsEngine
      ${OpenDynamicsEngine_LIBRARIES}
      ${sawOpenSceneGraph_LIBRARIES} )

    cisst_target_link_libraries(
      sawOpenDynamicsEngine
      ${REQUIRED_CISST_LIBRARIES} )

    configure_file (
      "${CMAKE_CURRENT_SOURCE_DIR}/sawOpenDynamicsEngineConfig.cmake.in"
      "${sawOpenDynamicsEngine_BINARY_DIR}/sawOpenDynamicsEngineConfig.cmake"
      @ONLY@)

    # Install target for headers and library
    install (DIRECTORY
             "${sawOpenDynamicsEngine_SOURCE_DIR}/include/sawOpenDynamicsEngine"
             DESTINATION include
             PATTERN .svn EXCLUDE)

    install (FILES ${HEADER_FILES}
      DESTINATION include/sawOpenDynamicsEngine)

    install (TARGETS sawOpenDynamicsEngine
      RUNTIME DESTINATION bin
      LIBRARY DESTINATION lib
      ARCHIVE DESTINATION lib)

    # Install cmake files for finding ODE and sawODE
    install (FILES "${sawOpenDynamicsEngine_BINARY_DIR}/sawOpenDynamicsEngineConfig.cmake"
      DESTINATION "${CISST_CMAKE_INSTALL_SUFFIX}/saw")

    install (FILES "${sawOpenDynamicsEngine_SOURCE_DIR}/code/cmake/Modules/FindOpenDynamicsEngine.cmake"
      DESTINATION ${CISST_CMAKE_MODULES_INSTALL_SUFFIX})

  endif( sawOpenSceneGraph_FOUND )
endif( cisst_FOUND_AS_REQUIRED AND ODE_FOUND )


