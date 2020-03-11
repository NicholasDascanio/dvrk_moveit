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
  cisstMultiTask )

find_package( cisst REQUIRED ${REQUIRED_CISST_LIBRARIES} )

if( cisst_FOUND_AS_REQUIRED )

  # load cisst configuration
  include( ${CISST_USE_FILE} )

  cisst_find_saw_component( sawCANBus REQUIRED)

  if( sawCANBus_FOUND AND CISST_HAS_LINUX_XENOMAI)

    add_definitions( ${sawCANBus_DEFINITIONS} )

    add_executable( osaRTSocketCANRecvExample     osaRTSocketCANRecvExample.cpp )
    add_executable( osaRTSocketCANSendExample     osaRTSocketCANSendExample.cpp )
    add_executable( mtsRTSocketCANSendRecvExample mtsRTSocketCANSendRecvExample.cpp )

    target_link_libraries( osaRTSocketCANRecvExample sawCANBus )
    target_link_libraries( osaRTSocketCANSendExample sawCANBus )
    target_link_libraries( mtsRTSocketCANSendRecvExample sawCANBus )

#    cisst_target_link_libraries( osaRTSocketCANRecvExample ${REQUIRED_CISST_LIBRARIES} )
#    cisst_target_link_libraries( osaRTSocketCANSendExample ${REQUIRED_CISST_LIBRARIES} )
#    cisst_target_link_libraries( mtsRTSocketCANSendRecvExample ${REQUIRED_CISST_LIBRARIES} )

  endif( sawCANBus_FOUND AND CISST_HAS_LINUX_XENOMAI)

endif( cisst_FOUND_AS_REQUIRED )
