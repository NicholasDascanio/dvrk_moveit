#
#
# (C) Copyright 2009-2012 Johns Hopkins University (JHU), All Rights
# Reserved.
#
# --- begin cisst license - do not edit ---
#
# This software is provided "as is" under an open source license, with
# no warranty.  The complete license can be found in license.txt and
# http://www.cisst.org/cisst/license.txt.
#
# --- end cisst license ---

# list of cisst libraries needed
set (REQUIRED_CISST_LIBRARIES cisstCommon cisstVector cisstOSAbstraction
                              cisstMultiTask cisstMultiTaskQt
                              cisstParameterTypes cisstQt)

# find cisst and make sure the required libraries have been compiled
find_package (cisst REQUIRED ${REQUIRED_CISST_LIBRARIES})

if (cisst_FOUND_AS_REQUIRED)

  # load cisst configuration
  include (${CISST_USE_FILE})

  # sawSartoriusScale has been compiled within cisst, we should find it automatically
  cisst_find_saw_component (sawSartoriusScale REQUIRED)

  if (sawSartoriusScale_FOUND)

    # sawSartoriusScale configuration
    include_directories (${sawSartoriusScale_INCLUDE_DIR})
    link_directories (${sawSartoriusScale_LIBRARY_DIR})

    # name the main executable and specifies with source files to use
    add_executable (sawSartoriusScaleExampleQt
                    main.cpp
                    )
    set_property (TARGET sawSartoriusScaleExampleQt PROPERTY FOLDER "sawSartoriusScale")
    # link against non cisst libraries and saw components
    target_link_libraries (sawSartoriusScaleExampleQt
                           ${sawSartoriusScale_LIBRARIES}
                           )
    # link against cisst libraries (and dependencies)
    cisst_target_link_libraries (sawSartoriusScaleExampleQt ${REQUIRED_CISST_LIBRARIES})

  endif (sawSartoriusScale_FOUND)

else (cisst_FOUND_AS_REQUIRED)
  message ("Information: code in ${CMAKE_CURRENT_SOURCE_DIR} will not be compiled, it requires ${REQUIRED_CISST_LIBRARIES}")
endif (cisst_FOUND_AS_REQUIRED)
