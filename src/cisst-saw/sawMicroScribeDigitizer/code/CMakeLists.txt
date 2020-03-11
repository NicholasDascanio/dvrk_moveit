#
#
# Author(s):  Min Yang Jung
# Created on: 2011-10-06
#
# (C) Copyright 2011 Johns Hopkins University (JHU), All Rights
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
                              cisstMultiTask)

find_package (cisst REQUIRED ${REQUIRED_CISST_LIBRARIES})

if (cisst_FOUND_AS_REQUIRED)

  # load cisst configuration
  include (${CISST_USE_FILE})

  # add definition for code
  add_library (sawMicroScribeDigitizer ${IS_SHARED}
               ../sawMicroScribeDigitizerExport.h
               mtsMicroScribeDigitizer.cpp
               ../mtsMicroScribeDigitizer.h
               )

  target_link_libraries (sawMicroScribeDigitizer ${MicroScribe_LIBRARIES})

  set (sawMicroScribeDigitizer_INCLUDE_DIR "${sawComponents_SOURCE_DIR}")
  set (sawMicroScribeDigitizer_LIBRARY_DIR "${LIBRARY_OUTPUT_PATH}")
  set (sawMicroScribeDigitizer_LIBRARIES sawMicroScribeDigitizer ${MicroScribe_LIBRARIES})

  configure_file ("${CMAKE_CURRENT_SOURCE_DIR}/sawMicroScribeDigitizerConfig.cmake.in"
                  "${sawComponents_BINARY_DIR}/sawMicroScribeDigitizerConfig.cmake"
                   @ONLY@)

  cisst_target_link_libraries (sawMicroScribeDigitizer ${REQUIRED_CISST_LIBRARIES})

else (cisst_FOUND_AS_REQUIRED)
  message ("Code in ${CMAKE_CURRENT_SOURCE_DIR} will not be compiled since it requires ${REQUIRED_CISST_LIBRARIES}")
endif (cisst_FOUND_AS_REQUIRED)
