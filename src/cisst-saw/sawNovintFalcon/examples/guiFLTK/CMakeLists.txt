#
#
# (C) Copyright 2005-2012 Johns Hopkins University (JHU), All Rights
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
set (REQUIRED_CISST_LIBRARIES cisstCommon cisstVector cisstOSAbstraction
                              cisstMultiTask cisstParameterTypes cisstFLTK)

# find cisst and make sure the required libraries have been compiled
find_package (cisst REQUIRED ${REQUIRED_CISST_LIBRARIES})

if (cisst_FOUND_AS_REQUIRED)

  # load cisst configuration
  include (${CISST_USE_FILE})

  # sawNovintFalcon has been compiled within cisst, we should find it automatically
  cisst_find_saw_component (sawNovintFalcon REQUIRED)

  if (sawNovintFalcon_FOUND)

    # sawNovintFalcon configuration
    include_directories (${sawNovintFalcon_INCLUDE_DIR})
    link_directories (${sawNovintFalcon_LIBRARY_DIR})

    # CMake command, generate .cxx and .h from .fl files
    fltk_wrap_ui (sawNovintHDLExampleFLTK displayUI.fl)

    # name the main executable and specifies with source files to use
    add_executable (sawNovintHDLExampleFLTK
                    displayTask.cpp
                    main.cpp
                    displayTask.h
                    ${sawNovintHDLExampleFLTK_FLTK_UI_SRCS}
                    )
    set_property (TARGET sawNovintHDLExampleFLTK PROPERTY FOLDER "sawNovintFalcon")
    target_link_libraries (sawNovintHDLExampleFLTK ${sawNovintFalcon_LIBRARIES})

    # link against cisst libraries (and dependencies)
    cisst_target_link_libraries (sawNovintHDLExampleFLTK ${REQUIRED_CISST_LIBRARIES})

  endif (sawNovintFalcon_FOUND)

else (cisst_FOUND_AS_REQUIRED)
  message ("Information: code in ${CMAKE_CURRENT_SOURCE_DIR} will not be compiled, it requires ${REQUIRED_CISST_LIBRARIES}")
endif (cisst_FOUND_AS_REQUIRED)
