#
#
# CMakeLists for sawLoPoMoCo
#
# (C) Copyright 2008-2012 Johns Hopkins University (JHU), All Rights
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
                              cisstCommonXML
                              cisstVector
                              cisstOSAbstraction
                              cisstMultiTask
                              cisstParameterTypes)

find_package (cisst REQUIRED ${REQUIRED_CISST_LIBRARIES})

if (cisst_FOUND_AS_REQUIRED)

  # load cisst configuration
  include (${CISST_USE_FILE})

 # create/configure file for find_package (sawLoPoMoCo)
  set (sawLoPoMoCo_INCLUDE_DIR "${sawLoPoMoCo_SOURCE_DIR}/include")
  set (sawLoPoMoCo_LIBRARY_DIR "${LIBRARY_OUTPUT_PATH}")
  set (sawLoPoMoCo_LIBRARIES sawLoPoMoCo)

  include_directories (${sawLoPoMoCo_INCLUDE_DIR})
  set (sawLoPoMoCo_HEADER_DIR "${sawLoPoMoCo_INCLUDE_DIR}/sawLoPoMoCo")
  add_library (sawLoPoMoCo
               mtsLoPoMoCo.cpp
               osaLoPoMoCoOffsets.h
               osaLoPoMoCoBoardIO.h
               ${sawLoPoMoCo_HEADER_DIR}/mtsLoPoMoCo.h
               ${sawLoPoMoCo_HEADER_DIR}/sawLoPoMoCoExport.h)
  set_property (TARGET sawLoPoMoCo PROPERTY FOLDER "sawLoPoMoCo")
  cisst_target_link_libraries (sawLoPoMoCo ${REQUIRED_CISST_LIBRARIES})

  configure_file ("${CMAKE_CURRENT_SOURCE_DIR}/sawLoPoMoCoConfig.cmake.in"
                  "${sawLoPoMoCo_BINARY_DIR}/sawLoPoMoCoConfig.cmake"
                   @ONLY@)

  # Install target for headers and library
  install (DIRECTORY
           ${sawLoPoMoCo_INCLUDE_DIR}/sawLoPoMoCo
           DESTINATION include
           PATTERN .svn EXCLUDE)

  install (TARGETS sawLoPoMoCo
           RUNTIME DESTINATION bin
           LIBRARY DESTINATION lib
           ARCHIVE DESTINATION lib)

  install (FILES "${sawLoPoMoCo_BINARY_DIR}/sawLoPoMoCoConfig.cmake"
           DESTINATION "${CISST_CMAKE_INSTALL_SUFFIX}/saw")

else (cisst_FOUND_AS_REQUIRED)
  message ("Information: code in ${CMAKE_CURRENT_SOURCE_DIR} will not be compiled, it requires ${REQUIRED_CISST_LIBRARIES}")
endif (cisst_FOUND_AS_REQUIRED)
