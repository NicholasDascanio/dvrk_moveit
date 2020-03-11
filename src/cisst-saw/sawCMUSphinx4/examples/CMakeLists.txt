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

set (REQUIRED_CISST_LIBRARIES cisstCommon cisstVector cisstOSAbstraction
                              cisstMultiTask cisstParameterTypes cisstQt)

# find cisst and make sure the required libraries have been compiled
find_package (cisst REQUIRED ${REQUIRED_CISST_LIBRARIES})

if (cisst_FOUND)

  # load cisst configuration
  include (${CISST_USE_FILE})

  # sawCMUSphinx4 has been compiled within cisst, we should find it automatically
  # find_package (sawCMUSphinx4 REQUIRED HINTS "${cisst_saw_BINARY_DIR}/saw/components")
  # temporary hack until sawCMUSphinx4Config.cmake is generated
  include_directories ("${sawCMUSphinx4_SOURCE_DIR}/include" "${sawCMUSphinx4_BINARY_DIR}")

  # example with Qt
  get_filename_component (JAVA_LIBRARY_DIR ${JAVA_AWT_LIBRARY} PATH)
  link_directories (${JAVA_LIBRARY_DIR}/xawt)

  add_executable (sawCMUSphinx4QtExample mainQt.cpp)
  target_link_libraries (sawCMUSphinx4QtExample
                         sawCMUSphinx4 sawCMUSphinx4Qt ${QT_LIBRARIES})
  cisst_target_link_libraries (sawCMUSphinx4QtExample ${REQUIRED_CISST_LIBRARIES})

endif (cisst_FOUND)
