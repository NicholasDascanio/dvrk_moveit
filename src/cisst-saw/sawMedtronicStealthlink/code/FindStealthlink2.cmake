#
# $Id: FindStealthlink.cmake 3593 2012-04-09 18:33:58Z adeguet1 $
#
# (C) Copyright 2011 Johns Hopkins University (JHU), All Rights
# Reserved.
#
# --- begin cisst license - do not edit ---
#
# This software is provided "as is" under an open source license, with
# no warranty.  The complete license can be found in license.txt and
# http://www.cisst.org/cisst/license.txt.
#
# --- end cisst license ---


find_path (Stealthlink2_INCLUDE_DIRS
           NAMES "Stealthlink/Stealthlink.h"
           DOC "Include directory, i.e. parent directory of directory \"StealthLink\"")

# Linux specific section, look for one of these
if (UNIX)
   #64 bit use Stealthlink2
   if(CMAKE_SIZEOF_VOID_P EQUAL 8)
      find_library (Stealthlink2_LIBRARY StealthLink libStealthLink
                   PATHS "${Stealthlink2_INCLUDE_DIRS}/i686-linux-gnu3"
                   DOC "Only use with Stealthlink2")
   endif(CMAKE_SIZEOF_VOID_P EQUAL 8)
endif(UNIX)

# make sure we have everything we need
set (Stealthlink2_FOUND FALSE)


if (Stealthlink2_LIBRARY)
    set (Stealthlink2_LIBRARIES ${Stealthlink2_LIBRARY})
endif (Stealthlink2_LIBRARY)


# set to true if one library found along with include directory
if (Stealthlink2_INCLUDE_DIRS AND Stealthlink2_LIBRARIES)
   set (Stealthlink2_FOUND TRUE)
   if (WIN32)
       message (SEND_ERROR "Windows is not currently supported")
   endif (WIN32)
   mark_as_advanced (Stealthlink2_INCLUDE_DIRS
		     Stealthlink2_LIBRARY
                     Stealthlink2_LIBRARIES)
endif (Stealthlink2_INCLUDE_DIRS AND Stealthlink2_LIBRARIES)
