#
# $Id: CMakeLists.txt 3034 2011-10-09 01:53:36Z adeguet1 $
#
# (C) Copyright 2009-2012 Johns Hopkins University (JHU), All Rights Reserved.
#
# --- begin cisst license - do not edit ---
#
# This software is provided "as is" under an open source license, with
# no warranty.  The complete license can be found in license.txt and
# http://www.cisst.org/cisst/license.txt.
#
# --- end cisst license ---

# List cisst libraries needed
set (REQUIRED_CISST_LIBRARIES cisstCommon
                              cisstVector
                              cisstOSAbstraction
                              cisstMultiTask
                              cisstParameterTypes
                              cisstRobot)

# find cisst and make sure the required libraries have been compiled
find_package (cisst REQUIRED ${REQUIRED_CISST_LIBRARIES})

if (cisst_FOUND_AS_REQUIRED)

  # load cisst configuration
  include (${CISST_USE_FILE})

  # sawGLUTSimulator has been compiled within cisst, we should find it automatically
  cisst_find_saw_component (sawGLUTSimulator REQUIRED)
  cisst_find_saw_component (sawKeyboard      REQUIRED)

  if (sawGLUTSimulator_FOUND AND sawKeyboard_FOUND)

    include_directories (${sawGLUTSimulator_INCLUDE_DIR})
    link_directories (${sawGLUTSimulator_LIBRARY_DIR})

    include_directories (${sawKeyboard_INCLUDE_DIR})
    link_directories (${sawKeyboard_LIBRARY_DIR})

    add_executable (sawGLUTSimulatorExWAM wamglut.cpp )
    target_link_libraries (sawGLUTSimulatorExWAM
                           ${sawGLUTSimulator_LIBRARIES}
                           ${sawKeyboard_LIBRARIES}
                           )
    cisst_target_link_libraries (sawGLUTSimulatorExWAM ${REQUIRED_CISST_LIBRARIES})
  endif (sawGLUTSimulator_FOUND AND sawKeyboard_FOUND)

endif (cisst_FOUND_AS_REQUIRED)
