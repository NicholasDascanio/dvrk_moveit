#
#
# (C) Copyright 2005-2011 Johns Hopkins University (JHU), All Rights
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
                              cisstMultiTask cisstParameterTypes)

# find cisst and make sure the required libraries have been compiled
find_package (cisst REQUIRED ${REQUIRED_CISST_LIBRARIES})

if (cisst_FOUND)

  # load cisst configuration
  include (${CISST_USE_FILE})

  # sawOpenAL has been compiled within cisst, we should find it automatically
  cisst_find_saw_component (sawOpenAL REQUIRED)

  if (sawOpenAL_FOUND)

    # sawOpenAL configuration
    include_directories (${sawOpenAL_INCLUDE_DIR})
    link_directories (${sawOpenAL_LIBRARY_DIR})

    add_executable (sawOpenALExample
                    main.cpp
                    mtsOpenALTestComponent.h
                    mtsOpenALTestComponent.cpp)

    # link against non cisst libraries and saw components
    target_link_libraries (sawOpenALExample
                           ${sawOpenAL_LIBRARIES}
                           )
    # link against cisst libraries (and dependencies)
    cisst_target_link_libraries (sawOpenALExample ${REQUIRED_CISST_LIBRARIES})

  endif (sawOpenAL_FOUND)

endif (cisst_FOUND)
