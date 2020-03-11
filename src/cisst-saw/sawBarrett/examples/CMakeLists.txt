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
  cisstParameterTypes
  cisstRobot
  cisstNumerical )

find_package(cisst REQUIRED ${REQUIRED_CISST_LIBRARIES})

if( cisst_FOUND  )

  # load cisst configuration
  include( ${CISST_USE_FILE} )
  
  cisst_find_saw_component(sawCANBus REQUIRED)
  cisst_find_saw_component(sawBarrett REQUIRED)
  
  if( sawCANBus_FOUND AND sawBarrett_FOUND )
    
    include_directories( ${sawCANBus_INCLUDE_DIR} ${sawBarrett_INCLUDE_DIR} )

    link_directories(${sawCANBus_LIBRARY_DIR})

    set( REQUIRED_SAW_COMPONENTS sawBarrett sawCANBus )

    set( sawBarrett_EXAMPLES
      osaPuckExample
      osaGroupExample
      osaWAMExample
      osaBH8_280Example
      mtsPuckExample
      mtsWAMExample )
      
    set( sawBarrett_INCLUDE_DIR
      ${sawKeyboard_INCLUDE_DIR}
      ${sawControllers_INCLUDE_DIR}
      ${sawTrajectories_INCLUDE_DIR} )

    set( sawBarrett_LIBRARY_DIR
      ${sawKeyboard_LIBRARY_DIR}
      ${sawControllers_LIBRARY_DIR}
      ${sawTrajectories_LIBRARY_DIR} )

    set( REQUIRED_SAW_COMPONENTS sawCANBus sawBarrett )
    
    cisst_find_saw_component( sawControllers )
    cisst_find_saw_component( sawTrajectories )
    cisst_find_saw_component( sawKeyboard )

    if( sawControllers_FOUND AND sawKeyboard_FOUND AND sawTrajectories_FOUND )

      set( sawBarrett_INCLUDE_DIR
	${sawBarrett_INCLUDE_DIR}
        ${sawKeyboard_INCLUDE_DIR}
        ${sawControllers_INCLUDE_DIR}
        ${sawTrajectories_INCLUDE_DIR} )
      
      set( sawBarrett_LIBRARY_DIR
	${sawBarrett_LIBRARY_DIR}
	${sawControllers_LIBRARY_DIR} 
	${sawKeyboard_LIBRARY_DIR}
	${sawTrajectories_LIBRARY_DIR} )
      
      set( REQUIRED_SAW_COMPONENTS
	${REQUIRED_SAW_COMPONENTS}
        sawControllers
        sawTrajectories
        sawKeyboard )
      
      set(sawBarrett_EXAMPLES
	${sawBarrett_EXAMPLES}
        osaWAMGCExample
        osaWAMPDGCExample
        mtsWAMGCExample
        mtsWAMPDGCExample
        mtsWAMMoveExample )
      
      
    endif( sawControllers_FOUND AND 
      sawKeyboard_FOUND AND 
      sawTrajectories_FOUND )

    include_directories( ${sawBarrett_INCLUDE_DIR} )
    link_directories( ${sawBarrett_LIBRARY_DIR} )

    foreach (_example ${sawBarrett_EXAMPLES})
      add_executable( ${_example} ${_example}.cpp )
      target_link_libraries( ${_example} ${REQUIRED_SAW_COMPONENTS} )
      cisst_target_link_libraries(${_example} ${REQUIRED_CISST_LIBRARIES})
    endforeach ()
    
  endif( sawCANBus_FOUND  AND sawBarrett_FOUND )
  
endif( cisst_FOUND )
