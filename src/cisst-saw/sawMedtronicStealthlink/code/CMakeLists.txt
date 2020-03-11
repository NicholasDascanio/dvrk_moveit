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

set (REQUIRED_CISST_LIBRARIES cisstCommon
                              cisstCommonXML
                              cisstVector
                              cisstOSAbstraction
                              cisstMultiTask
                              cisstParameterTypes)

find_package (cisst REQUIRED ${REQUIRED_CISST_LIBRARIES})

if (cisst_FOUND_AS_REQUIRED)

  # load cisst configuration
  include (${CISST_USE_FILE})

  set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR})
  find_package (Stealthlink )

  find_package (Stealthlink2 )

  if (Stealthlink_FOUND OR Stealthlink2_FOUND)

    if (Stealthlink_FOUND AND Stealthlink2_FOUND)
        option (CISST_HAS_STEALTHLINK "Is MedtronicStealthlink available?" OFF)
            message (SEND_ERROR "Only one version of Stealthlink is support at a time.  Please select the locations for only one version.")
    endif (Stealthlink_FOUND AND Stealthlink2_FOUND)

    # create alias for code and cmake configured file
    option (SAW_MedtronicStealthlink_SIMULATOR "Wrappers for Medtronic StealthLink simulator (build instead of real wrapper which requires binaries from Medtronic)" OFF)
    mark_as_advanced (SAW_MedtronicStealthlink_SIMULATOR)
    set (sawMedtronicStealthlink_IS_SIMULATOR ${SAW_MedtronicStealthlink_SIMULATOR})

    option (CISST_HAS_STEALTHLINK "Is MedtronicStealthlink available?" ON)
    mark_as_advanced (CISST_HAS_STEALTHLINK)
    if (CISST_HAS_STEALTHLINK)
        add_definitions (-DCISST_HAS_STEALTHLINK)
        message ("Added -DCISST_HAS_STEALTHLINK")
    endif (CISST_HAS_STEALTHLINK)
    
    # add definition for code
    if (sawMedtronicStealthlink_IS_SIMULATOR)
      add_definitions (-DsawMedtronicStealthlink_IS_SIMULATOR)
    endif (sawMedtronicStealthlink_IS_SIMULATOR)

    set (sawMedtronicStealthlink_INCLUDE_DIR "${sawMedtronicStealthlink_SOURCE_DIR}/include")
    set (sawMedtronicStealthlink_LIBRARY_DIR "${LIBRARY_OUTPUT_PATH}")

    set (sawMedtronicStealthlink_HEADER_DIR "${sawMedtronicStealthlink_SOURCE_DIR}/include/sawMedtronicStealthlink")

    if(Stealthlink_FOUND)
    set (sawMedtronicStealthlink_LIBRARIES sawMedtronicStealthlink ${Stealthlink_LIBRARIES})
    include_directories (${sawMedtronicStealthlink_INCLUDE_DIR} ${Stealthlink_INCLUDE_DIRS} )
    set (mtsMedtronicStealthlink_SOURCES mtsMedtronicStealthlink.cpp mtsMedtronicStealthlinkTypes.cpp ${sawMedtronicStealthlink_HEADER_DIR}/mtsMedtronicStealthlink.h ${sawMedtronicStealthlink_HEADER_DIR}/mtsMedtronicStealthlinkTypes.h)
    set (mtsMedtronicStealthlink_AsCL_Stuff_HEADER mtsMedtronicStealthlink_AsCL_Stuff.h mtsMedtronicStealthlink_AsCL_Stuff.cpp)
    endif(Stealthlink_FOUND)

    if(Stealthlink2_FOUND)
    set (sawMedtronicStealthlink_LIBRARIES sawMedtronicStealthlink ${Stealthlink2_LIBRARIES})
    include_directories (${sawMedtronicStealthlink_INCLUDE_DIR} ${Stealthlink2_INCLUDE_DIRS} )
    set (mtsMedtronicStealthlink_SOURCES mtsMedtronicStealthlink2.cpp ${sawMedtronicStealthlink_HEADER_DIR}/mtsMedtronicStealthlink2.h)
    set (mtsMedtronicStealthlink_AsCL_Stuff_HEADER) 
    endif(Stealthlink2_FOUND)

    
    add_library (sawMedtronicStealthlink ${IS_SHARED}
                 ${sawMedtronicStealthlink_HEADER_DIR}/sawMedtronicStealthlinkExport.h
                 ${mtsMedtronicStealthlink_SOURCES}
                 ${mtsMedtronicStealthlink_AsCL_Stuff_HEADER}
                 )
    set_property (TARGET sawMedtronicStealthlink PROPERTY FOLDER "sawMedtronicStealthlink")

    if(Stealthlink_FOUND)
        target_link_libraries (sawMedtronicStealthlink ${Stealthlink_LIBRARIES})
    endif(Stealthlink_FOUND)

    if(Stealthlink2_FOUND)
        target_link_libraries (sawMedtronicStealthlink ${Stealthlink2_LIBRARIES})
    endif(Stealthlink2_FOUND)

    if (WIN32)
      target_link_libraries (sawMedtronicStealthlink ws2_32.lib)
      set (sawMedtronicStealthlink_LIBRARIES ${sawMedtronicStealthlink_LIBRARIES} ws2_32.lib)
      set (CMAKE_SHARED_LINKER_FLAGS_RELEASE        "${CMAKE_SHARED_LINKER_FLAGS_RELEASE}         /NODEFAULTLIB:msvcprt.lib /NODEFAULTLIB:LIBCMT.lib /ignore:4049 /ignore:4217")
      set (CMAKE_SHARED_LINKER_FLAGS_MINSIZEREL     "${CMAKE_SHARED_LINKER_FLAGS_MINSIZEREL}      /NODEFAULTLIB:msvcprt.lib /NODEFAULTLIB:LIBCMT.lib /ignore:4049 /ignore:4217")
      set (CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO}  /NODEFAULTLIB:msvcprt.lib /NODEFAULTLIB:LIBCMT.lib /ignore:4049 /ignore:4217")
      set (CMAKE_SHARED_LINKER_FLAGS_DEBUG          "${CMAKE_SHARED_LINKER_FLAGS_DEBUG}           /NODEFAULTLIB:msvcprtd.lib /NODEFAULTLIB:LIBCMTD.lib /ignore:4049 /ignore:4217")
    endif (WIN32)

    cisst_target_link_libraries (sawMedtronicStealthlink ${REQUIRED_CISST_LIBRARIES})

    configure_file ("${CMAKE_CURRENT_SOURCE_DIR}/sawMedtronicStealthlinkConfig.cmake.in"
                    "${sawMedtronicStealthlink_BINARY_DIR}/sawMedtronicStealthlinkConfig.cmake"
                     @ONLY)

    # Install target for headers and library
    install (DIRECTORY
             ${sawMedtronicStealthlink_INCLUDE_DIR}/sawMedtronicStealthlink
             DESTINATION include
             PATTERN .svn EXCLUDE)

    install (TARGETS sawMedtronicStealthlink
             RUNTIME DESTINATION bin
             LIBRARY DESTINATION lib
             ARCHIVE DESTINATION lib)

    install (FILES "${sawMedtronicStealthlink_BINARY_DIR}/sawMedtronicStealthlinkConfig.cmake"
             DESTINATION "${CISST_CMAKE_INSTALL_SUFFIX}/saw")

  else (Stealthlink_FOUND OR Stealthlink2_FOUND)
    option (CISST_HAS_STEALTHLINK "Is MedtronicStealthlink available?" OFF)
    message (SEND_ERROR "code in ${CMAKE_CURRENT_SOURCE_DIR} requires Stealthlink.")
  endif (Stealthlink_FOUND OR Stealthlink2_FOUND)

else (cisst_FOUND_AS_REQUIRED)
  message ("Information: code in ${CMAKE_CURRENT_SOURCE_DIR} will not be compiled, it requires ${REQUIRED_CISST_LIBRARIES}")
endif (cisst_FOUND_AS_REQUIRED)
