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

# this example is available on Linux only for now
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

  # list of cisst libraries needed
  set (REQUIRED_CISST_LIBRARIES
       cisstCommon
       cisstVector
       cisstOSAbstraction
       cisstMultiTask)

  # find cisst and make sure the required libraries have been compiled
  find_package (cisst REQUIRED ${REQUIRED_CISST_LIBRARIES})

  if (cisst_FOUND_AS_REQUIRED)

    # load cisst configuration
    include (${CISST_USE_FILE})

    # saw3Dconnexion has been compiled within cisst, we should find it automatically
    cisst_find_saw_component (saw3Dconnexion REQUIRED)

    if (saw3Dconnexion_FOUND)

      # saw3Dconnexion configuration
      include_directories (${saw3Dconnexion_INCLUDE_DIR})
      link_directories (${saw3Dconnexion_LIBRARY_DIR})

      # name the main executable and specifies with source files to use
      add_executable (osa3DconnexionExample osa3DconnexionExample.cpp)

      # link against non cisst libraries and saw components
      target_link_libraries (osa3DconnexionExample ${saw3Dconnexion_LIBRARIES})
      # link against cisst libraries (and dependencies)
      cisst_target_link_libraries (osa3DconnexionExample ${REQUIRED_CISST_LIBRARIES})

    endif (saw3Dconnexion_FOUND)

  else (cisst_FOUND_AS_REQUIRED)
    message ("Information: code in ${CMAKE_CURRENT_SOURCE_DIR} will not be compiled, it requires ${REQUIRED_CISST_LIBRARIES}")
  endif (cisst_FOUND_AS_REQUIRED)

else (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  message ("Information: code in ${CMAKE_CURRENT_SOURCE_DIR} will not be compiled, it requires Linux")
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
