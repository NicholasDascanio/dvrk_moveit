#
# (C) Copyright 2011-2015 Johns Hopkins University (JHU), All Rights Reserved.
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
                              cisstParameterTypes)

find_package (cisst REQUIRED ${REQUIRED_CISST_LIBRARIES})

if (cisst_FOUND_AS_REQUIRED)

  # load cisst configuration
  include (${CISST_USE_FILE})

  # where to save the libraries
  if (NOT DEFINED LIBRARY_OUTPUT_PATH)
    set (LIBRARY_OUTPUT_PATH "${sawNovintFalcon_BINARY_DIR}/lib")
  endif ()

  set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}")
  find_package (NovintHDL REQUIRED)

  if (NOVINT_FOUND)

    set (sawNovintFalcon_INCLUDE_DIR "${sawNovintFalcon_SOURCE_DIR}/include"
                                     "${sawNovintFalcon_BINARY_DIR}/include")
    set (sawNovintFalcon_LIBRARY_DIR "${LIBRARY_OUTPUT_PATH}")
    set (sawNovintFalcon_LIBRARIES sawNovintFalcon ${NOVINT_LIBRARIES})

    # Set the version number
    set (sawNovintFalcon_VERSION_MAJOR "1")
    set (sawNovintFalcon_VERSION_MINOR "0")
    set (sawNovintFalcon_VERSION_PATCH "1")
    set (sawNovintFalcon_VERSION "${sawNovintFalcon_VERSION_MAJOR}.${sawNovintFalcon_VERSION_MINOR}.${sawNovintFalcon_VERSION_PATCH}")

    # Generate sawNovintFalconRevision.h
    configure_file ("${sawNovintFalcon_SOURCE_DIR}/code/sawNovintFalconRevision.h.in"
                    "${sawNovintFalcon_BINARY_DIR}/include/sawNovintFalcon/sawNovintFalconRevision.h")

    include_directories (${NOVINT_INCLUDE_DIR} ${sawNovintFalcon_INCLUDE_DIR})
    set (sawNovintFalcon_HEADER_DIR "${sawNovintFalcon_SOURCE_DIR}/include/sawNovintFalcon")
    add_library (sawNovintFalcon ${IS_SHARED}
                 ${sawNovintFalcon_HEADER_DIR}/sawNovintFalconExport.h
                 mtsNovintHDL.cpp
                 ${sawNovintFalcon_HEADER_DIR}/mtsNovintHDL.h
                 )
    set_property (TARGET sawNovintFalcon PROPERTY FOLDER "sawNovintFalcon")
    target_link_libraries (sawNovintFalcon ${NOVINT_LIBRARIES})

    # add Qt code
#    add_subdirectory (Qt)
#    set (sawNovintFalcon_LIBRARIES ${sawNovintFalcon_LIBRARIES} ${sawNovintFalconQt_LIBRARIES})

    configure_file ("${CMAKE_CURRENT_SOURCE_DIR}/sawNovintFalconConfig.cmake.in"
                    "${sawNovintFalcon_BINARY_DIR}/sawNovintFalconConfig.cmake"
                     @ONLY@)

    cisst_target_link_libraries (sawNovintFalcon ${REQUIRED_CISST_LIBRARIES})

    # Install target for headers and library
    install (DIRECTORY
             ${sawNovintFalcon_SOURCE_DIR}/include/sawNovintFalcon
             ${sawNovintFalcon_BINARY_DIR}/include/sawNovintFalcon
             DESTINATION include
             PATTERN .svn EXCLUDE)

    install (TARGETS sawNovintFalcon
             RUNTIME DESTINATION bin
             LIBRARY DESTINATION lib
             ARCHIVE DESTINATION lib)

    install (FILES "${sawNovintFalcon_BINARY_DIR}/sawNovintFalconConfig.cmake"
             DESTINATION "${CISST_CMAKE_INSTALL_SUFFIX}/saw")

  else (NOVINT_FOUND)
    message (SEND_ERROR "code in ${CMAKE_CURRENT_SOURCE_DIR} requires NovintHDL.")
  endif (NOVINT_FOUND)

else (cisst_FOUND_AS_REQUIRED)
  message ("Information: code in ${CMAKE_CURRENT_SOURCE_DIR} will not be compiled, it requires ${REQUIRED_CISST_LIBRARIES}")
endif (cisst_FOUND_AS_REQUIRED)
