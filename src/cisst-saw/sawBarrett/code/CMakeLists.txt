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

set(
  REQUIRED_CISST_LIBRARIES
  cisstCommon
  cisstVector
  cisstOSAbstraction
  cisstMultiTask
  cisstParameterTypes )

find_package( cisst REQUIRED ${REQUIRED_CISST_LIBRARIES} )

if( cisst_FOUND )

  # load cisst configuration
  include (${CISST_USE_FILE})

  cisst_find_saw_component( sawCANBus REQUIRED )

  if( sawCANBus_FOUND )

    # include sawCANBus
    include_directories(${sawCANBus_INCLUDE_DIR})
    link_directories(${sawCANBus_LIBRARY_DIR})

    # create/configure file for find_package (sawBarret)
    set(sawBarrett_INCLUDE_DIR "${sawBarrett_SOURCE_DIR}/include")
    set(sawBarrett_LIBRARY_DIR "${LIBRARY_OUTPUT_PATH}")
    set(sawBarrett_LIBRARIES sawBarrett)

    include_directories (${sawBarrett_INCLUDE_DIR})
    set (sawBarrett_HEADER_DIR "${sawBarrett_INCLUDE_DIR}/sawBarrett")

    message(STATUS "sawBarrett_SOURCE_DIR = " ${sawBarrett_SOURCE_DIR})
    message(STATUS "sawBarrett_INCLUDE_DIR = " ${sawBarrett_INCLUDE_DIR})

    set(
      HEADER_FILES
      ${sawBarrett_HEADER_DIR}/Barrett.h
      ${sawBarrett_HEADER_DIR}/osaPuck.h
      ${sawBarrett_HEADER_DIR}/osaGroup.h
      ${sawBarrett_HEADER_DIR}/osaWAM.h
      ${sawBarrett_HEADER_DIR}/osaBH8_280.h
      ${sawBarrett_HEADER_DIR}/mtsPuck.h
      ${sawBarrett_HEADER_DIR}/mtsWAM.h )

    set(
      SOURCE_FILES
      osaPuck.cpp
      osaGroup.cpp
      osaWAM.cpp
      osaBH8_280.cpp
      mtsPuck.cpp
      mtsWAM.cpp )


    add_library( sawBarrett ${HEADER_FILES} ${SOURCE_FILES} )
    cisst_target_link_libraries( sawBarrett ${REQUIRED_CISST_LIBRARIES} )    

    configure_file(
      "${CMAKE_CURRENT_SOURCE_DIR}/sawBarrettConfig.cmake.in"
      "${sawBarrett_BINARY_DIR}/sawBarrettConfig.cmake"
      @ONLY@ )

    # Install targets for headers and library
    install( FILES ${HEADER_FILES}
      DESTINATION include/sawBarrett)

    install( TARGETS sawBarrett
      RUNTIME DESTINATION bin
      LIBRARY DESTINATION lib
      ARCHIVE DESTINATION lib)

    install( FILES "${sawComponents_BINARY_DIR}/sawBarrettConfig.cmake"
      DESTINATION "${CISST_CMAKE_INSTALL_SUFFIX}/saw")
  endif(sawCANBus_FOUND)
  
endif( cisst_FOUND )
