#
#
# (C) Copyright 2011-2012 Johns Hopkins University (JHU), All Rights Reserved.
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

# find cisst and make sure the required libraries have been compiled
find_package (cisst REQUIRED ${REQUIRED_CISST_LIBRARIES})

if (cisst_FOUND)

  # load cisst configuration
  include (${CISST_USE_FILE})

  # sawMedtronicStealthlink has been compiled within cisst, we should find it automatically
  cisst_find_saw_component (sawMedtronicStealthlink REQUIRED)

  if (sawMedtronicStealthlink_FOUND)

    # sawMedtronicStealthlink configuration
    include_directories (${sawMedtronicStealthlink_INCLUDE_DIR})
    link_directories (${sawMedtronicStealthlink_LIBRARY_DIR})

    add_executable (sawMedtronicStealthlinkExample
                    mtsMedtronicStealthlinkExampleComponent.h
                    mtsMedtronicStealthlinkExampleComponent.cpp
                    main.cpp)
    set_property (TARGET sawMedtronicStealthlinkExample PROPERTY FOLDER "sawMedtronicStealthlink")
    target_link_libraries (sawMedtronicStealthlinkExample
                           sawMedtronicStealthlink ${sawMedtronicStealthlink_LIBRARIES})
    cisst_target_link_libraries (sawMedtronicStealthlinkExample ${REQUIRED_CISST_LIBRARIES})

    # compile distributed example is ZeroC Ice is available
    if (CISST_MTS_HAS_ICE)
      add_executable (sawMedtronicStealthlinkExampleServer
                      mainServer.cpp)
      set_property (TARGET sawMedtronicStealthlinkExampleServer PROPERTY FOLDER "sawMedtronicStealthlink")
      target_link_libraries (sawMedtronicStealthlinkExampleServer
                             sawMedtronicStealthlink ${sawMedtronicStealthlink_LIBRARIES})
      cisst_target_link_libraries (sawMedtronicStealthlinkExampleServer ${REQUIRED_CISST_LIBRARIES})

      add_executable (sawMedtronicStealthlinkExampleClient
                      mtsMedtronicStealthlinkExampleComponent.h
                      mtsMedtronicStealthlinkExampleComponent.cpp
                      mainClient.cpp)
      set_property (TARGET sawMedtronicStealthlinkExampleClient PROPERTY FOLDER "sawMedtronicStealthlink")
      target_link_libraries (sawMedtronicStealthlinkExampleClient
                             sawMedtronicStealthlink ${sawMedtronicStealthlink_LIBRARIES})
      cisst_target_link_libraries (sawMedtronicStealthlinkExampleClient ${REQUIRED_CISST_LIBRARIES})
    endif (CISST_MTS_HAS_ICE)

  endif (sawMedtronicStealthlink_FOUND)

endif (cisst_FOUND)
