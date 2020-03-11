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
  cisstRobot
  cisstParameterTypes )

find_package( cisst REQUIRED ${REQUIRED_CISST_LIBRARIES} )

if( cisst_FOUND_AS_REQUIRED )

  include( ${CISST_USE_FILE} )
  cisst_find_saw_component( sawOpenDynamicsEngine QUIET REQUIRED )

  if( sawOpenDynamicsEngine_FOUND )
    include_directories( ${sawOpenDynamicsEngine_INCLUDE_DIRS} )
    link_directories( ${sawOpenDynamicsEngine_LIBRARY_DIR} )
    add_definitions( "${sawOpenDynamicsEngine_DEFINITIONS} -DCISST_ODE_SUPPORT" )

    set (sawOpenDynamicsEngine_EXAMPLES
         osaODEBodyExample
         osaODEWAMExample
         osaODEWAMBarrettHandExample
         osaODEBarrettHandExample

         mtsODEWAMExample
         mtsODEWAMBarrettHandExample
         mtsODEBarrettHandExample)

    foreach (_example ${sawOpenDynamicsEngine_EXAMPLES})
      add_executable (${_example} ${_example}.cpp)
      target_link_libraries (${_example} ${sawOpenDynamicsEngine_LIBRARIES})
      cisst_target_link_libraries (${_example} ${REQUIRED_CISST_LIBRARIES})
      set_property (TARGET ${_example} PROPERTY FOLDER "sawOpenDynamicsEngine")
    endforeach ()

  endif( sawOpenDynamicsEngine_FOUND)
endif( cisst_FOUND_AS_REQUIRED )
