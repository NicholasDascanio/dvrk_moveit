#
# $Id$
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

if (WIN32)

  find_path( ODE_INCLUDE_DIR ode/ode.h
    PATHS ENV ODE_ROOT_PATH PATH_SUFFIXES include )

  find_library( ODE_LIBRARY NAMES ode
    PATHS ENV ODE_ROOT_PATH PATH_SUFFIXES lib )

elseif (UNIX)

  find_path( ODE_INCLUDE_DIR ode/ode.h )

  # On RedHat, the ode double-precision library is called "ode-double" and
  # the single-precision is called "ode" but on Debian/Ubuntu they are
  # called "ode" and "ode-sp", repsectively
  find_library( ODE_LIBRARY ode-double ode )

  if (ODE_LIBRARY STREQUAL "ODE_LIBRARY-NOTFOUND")
    find_library( ODE_LIBRARY ode )
  endif ()

endif ()

set( ODE_FOUND FALSE )

if( ODE_INCLUDE_DIR )
  if( ODE_LIBRARY )
    set( ODE_FOUND TRUE )

    # deprecate old variables so that we conform to standard variable naming for Find*.cmake
    set( OpenDynamicsEngine_FOUND True )
    set( OpenDynamicsEngine_LIBRARIES ${ODE_LIBRARY} )
    set( OpenDynamicsEngine_INCLUDE_DIRS ${ODE_INCLUDE_DIR} )
  endif()
endif()

mark_as_advanced( ODE_INCLUDE_DIR ODE_LIBRARY )
