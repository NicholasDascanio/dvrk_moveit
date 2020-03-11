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
                              cisstVector
                              cisstOSAbstraction
                              cisstMultiTask
                              cisstParameterTypes)

find_package (cisst REQUIRED ${REQUIRED_CISST_LIBRARIES})

if (cisst_FOUND_AS_REQUIRED)

  # load cisst configuration
  include (${CISST_USE_FILE})

  # find and use JNI
  find_package (JNI REQUIRED)
  include_directories (${JNI_INCLUDE_DIRS})

  # find Java
  find_package (Java REQUIRED)

  # find Sphinx 4
  find_path (SAW_SPHINX4_DIST_DIR
             NAMES lib/sphinx4.jar
             DOC "Directory containing binary distribution of sphynx 4 (beta 6)")

  # Java simplified wrapper for sphinx4
  set (SAW_SPHINX4_JAVACLASS_DIR ${LIBRARY_OUTPUT_PATH})
  set (SAW_SPHINX4_JAVA ${SAW_SPHINX4_JAVACLASS_DIR}/sawCMUSphinx4.class)
  add_custom_command (OUTPUT ${SAW_SPHINX4_JAVA}
                      COMMAND ${JAVA_COMPILE}
                      ARGS -classpath "${SAW_SPHINX4_DIST_DIR}/lib/sphinx4.jar"
                           -d "${SAW_SPHINX4_JAVACLASS_DIR}"
                           "${CMAKE_CURRENT_SOURCE_DIR}/sawCMUSphinx4.java"
                      DEPENDS sawCMUSphinx4.java)

  # compile all Java based targets
  add_custom_target (sawCMUSphinx4Java ALL DEPENDS ${SAW_SPHINX4_JAVA})

  # copy template file to binary dir
  set (SAW_SPHINX4_TEMPLATE_DIR ${LIBRARY_OUTPUT_PATH})
  file (COPY sawCMUSphinx4.xml.template DESTINATION ${SAW_SPHINX4_TEMPLATE_DIR})

  # create configuration file with all wrappers
  configure_file (mtsCMUSphinx4Config.h.in "${sawCMUSphinx4_BINARY_DIR}/sawCMUSphinx4/mtsCMUSphinx4Config.h" @ONLY)

  # core library
  set (sawCMUSphinx4_INCLUDE_DIR "${sawCMUSphinx4_SOURCE_DIR}/include" "${sawCMUSphinx4_BINARY_DIR}")
  set (sawCMUSphinx4_LIBRARY_DIR "${LIBRARY_OUTPUT_PATH}")
  set (sawCMUSphinx4_LIBRARIES sawCMUSphinx4)

  include_directories (${sawCMUSphinx4_INCLUDE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
  set (sawCMUSphinx4_HEADER_DIR "${sawCMUSphinx4_SOURCE_DIR}/include/sawCMUSphinx4")

  add_library (sawCMUSphinx4 SHARED
               ${sawCMUSphinx4_HEADER_DIR}/mtsCMUSphinx4Export.h
               ${sawCMUSphinx4_HEADER_DIR}/mtsCMUSphinx4.h
               mtsCMUSphinx4Context.cpp
               mtsCMUSphinx4WordActions.cpp
               mtsCMUSphinx4JNI.h
               mtsCMUSphinx4JNI.cpp
               mtsCMUSphinx4.cpp)
  add_dependencies (sawCMUSphinx4
                    sawCMUSphinx4Java)
  target_link_libraries (sawCMUSphinx4
                         ${JNI_LIBRARIES})
  cisst_target_link_libraries (sawCMUSphinx4 ${REQUIRED_CISST_LIBRARIES})

  # Qt wrappers
  if (CISST_HAS_QT)
    cisst_set_directories (cisstQt)

    qt4_wrap_cpp (QT_WRAPPED_CPP
                  ${sawCMUSphinx4_HEADER_DIR}/mtsCMUSphinx4QtComponent.h
                  ${sawCMUSphinx4_HEADER_DIR}/mtsCMUSphinx4QtWidget.h)

    add_library (sawCMUSphinx4Qt SHARED
                 ${sawCMUSphinx4_HEADER_DIR}/mtsCMUSphinx4ExportQt.h
                 ${sawCMUSphinx4_HEADER_DIR}/mtsCMUSphinx4QtComponent.h
                 ${sawCMUSphinx4_HEADER_DIR}/mtsCMUSphinx4QtWidget.h
                 mtsCMUSphinx4QtComponent.cpp
                 mtsCMUSphinx4QtWidget.cpp
                 ${QT_WRAPPED_CPP})

    target_link_libraries (sawCMUSphinx4Qt ${QT_LIBRARIES})
    cisst_target_link_libraries (sawCMUSphinx4Qt ${REQUIRED_CISST_LIBRARIES})

  endif (CISST_HAS_QT)

endif (cisst_FOUND_AS_REQUIRED)
