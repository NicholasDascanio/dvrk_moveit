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


# create a list of libraries needed for this project
set (REQUIRED_CISST_LIBRARIES cisstCommon
                              cisstVector
                              cisstOSAbstraction
                              cisstMultiTask
                              cisstFLTK)

# find cisst and make sure the required libraries have been compiled
find_package (cisst REQUIRED ${REQUIRED_CISST_LIBRARIES} QUIET)

if (cisst_FOUND_AS_REQUIRED)

  # load cisst configuration
  include (${CISST_USE_FILE})

  # sawMicroScribeDigitizer has been compiled within cisst, we should find it automatically
  cisst_find_saw_component (sawMicroScribeDigitizer REQUIRED)

  if (sawMicroScribeDigitizer_FOUND)

    # sawMicroScribeDigitizer configuration
    include_directories (${sawMicroScribeDigitizer_INCLUDE_DIR})
    link_directories (${sawMicroScribeDigitizer_LIBRARY_DIR})

    #
    # Console-based example application
    #
    add_executable (sawMicroScribeDigitizerExample DigitizerUserComponent.h
                                                   DigitizerUserComponent.cpp
                                                   main.cpp)

    # link against non cisst libraries and cisst components
    target_link_libraries (sawMicroScribeDigitizerExample ${sawMicroScribeDigitizer_LIBRARIES})

    # link against cisst libraries (and dependencies)
    cisst_target_link_libraries (sawMicroScribeDigitizerExample ${REQUIRED_CISST_LIBRARIES})

    #
    # FLTK-based example application
    #
    # add include paths
    include_directories (${sawMicroScribeDigitizerExampleFLTK_SOURCE_DIR})

    fltk_wrap_ui (sawMicroScribeDigitizerExampleFLTK DigitizerExampleUI.fl)

    add_executable (sawMicroScribeDigitizerExampleFLTK DigitizerUserComponent.h
                                                       DigitizerUserComponent.cpp
                                                       DigitizerUserComponentFLTK.h
                                                       DigitizerUserComponentFLTK.cpp
                                                       mainFLTK.cpp
                                                       ${sawMicroScribeDigitizerExampleFLTK_FLTK_UI_SRCS}
                                                       )

    # link against non cisst libraries and cisst components
    target_link_libraries (sawMicroScribeDigitizerExampleFLTK ${sawMicroScribeDigitizer_LIBRARIES})

    # link against cisst libraries (and dependencies)
    cisst_target_link_libraries (sawMicroScribeDigitizerExampleFLTK ${REQUIRED_CISST_LIBRARIES})
  endif (sawMicroScribeDigitizer_FOUND)
else (cisst_FOUND_AS_REQUIRED)
  cisst_information_message_missing_libraries (${REQUIRED_CISST_LIBRARIES})
endif (cisst_FOUND_AS_REQUIRED)
