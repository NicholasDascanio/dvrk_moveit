#
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

# Core library
set (REQUIRED_CISST_LIBRARIES cisstCommon
                              cisstVector
                              cisstOSAbstraction
                              cisstMultiTask
                              cisstParameterTypes)

find_package (cisst REQUIRED ${REQUIRED_CISST_LIBRARIES})
find_package (OpenAL REQUIRED)

if (cisst_FOUND_AS_REQUIRED)

  # load cisst configuration
  include (${CISST_USE_FILE})

  set (sawOpenAL_HEADER_DIR "${sawOpenAL_SOURCE_DIR}/include/sawOpenAL")
  # create/configure file for find_package (sawOpenAL)
  set (sawOpenAL_INCLUDE_DIR "${sawOpenAL_SOURCE_DIR}/include" "${sawComponents_SOURCE_DIR}" "${sawComponents_BINARY_DIR}" "${OPENAL_INCLUDE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}")
  set (sawOpenAL_LIBRARY_DIR "${LIBRARY_OUTPUT_PATH}")
  set (sawOpenAL_LIBRARIES sawOpenAL "${OPENAL_LIBRARY}")
  set (sawOpenALQt_LIBRARIES ${sawOpenAL_LIBRARIES} sawOpenALQt)

  include_directories (${sawOpenAL_INCLUDE_DIR})

  add_library (sawOpenAL
               ${sawOpenAL_HEADER_DIR}/sawOpenAL.h
               ${sawOpenAL_HEADER_DIR}/sawOpenALExport.h
               ${sawOpenAL_HEADER_DIR}/osaOpenALAudioTypes.h
               osaOpenAL.cpp
               mtsOpenALPlay.cpp
               ${sawOpenAL_HEADER_DIR}/mtsOpenALPlay.h
               mtsOpenALRecord.cpp
               ${sawOpenAL_HEADER_DIR}/mtsOpenALRecord.h)

  target_link_libraries (sawOpenAL "${OPENAL_LIBRARY}")
  cisst_target_link_libraries (sawOpenAL ${REQUIRED_CISST_LIBRARIES})

  # Qt widgets
  if (CISST_HAS_QT)

    # initial find_package doesn't have cisstQt
    cisst_set_directories (cisstQt)

    # for Qt generated header files
    include_directories (${CMAKE_CURRENT_BINARY_DIR}
                        "${sawOpenAL_SOURCE_DIR}/include")
    
    # Qt4 widgets
    if (CISST_HAS_QT4)

      qt4_wrap_cpp (SAW_OPENAL_QT_COMPONENTS_GENERATED
                    ${sawOpenAL_HEADER_DIR}/mtsOpenALPlayQtComponent.h
                    ${sawOpenAL_HEADER_DIR}/mtsOpenALRecordQtComponent.h
                    ${sawOpenAL_HEADER_DIR}/sawNoteRecorderQtComponent.h)
      qt4_wrap_ui (SAW_OPENAL_QT_WIDGETS_GENERATED
                   mtsOpenALPlayQtWidget.ui
                   mtsOpenALRecordQtWidget.ui
                   sawNoteRecorderQtWidget.ui)

    # Qt5 widgets
    elseif (CISST_HAS_QT5)
      qt5_wrap_cpp (SAW_OPENAL_QT_COMPONENTS_GENERATED
                  ${sawOpenAL_HEADER_DIR}/mtsOpenALPlayQtComponent.h
                  ${sawOpenAL_HEADER_DIR}/mtsOpenALRecordQtComponent.h
                  ${sawOpenAL_HEADER_DIR}/sawNoteRecorderQtComponent.h)
      qt5_wrap_ui (SAW_OPENAL_QT_WIDGETS_GENERATED
                 mtsOpenALPlayQtWidget.ui
                 mtsOpenALRecordQtWidget.ui
                 sawNoteRecorderQtWidget.ui)
    endif (CISST_HAS_QT4)

    add_library (sawOpenALQt
                 ${SAW_OPENAL_QT_COMPONENTS_GENERATED}
                 ${SAW_OPENAL_QT_WIDGETS_GENERATED}
                 ${sawOpenAL_HEADER_DIR}/sawOpenALExportQt.h
                 mtsOpenALPlayQtComponent.cpp
                 ${sawOpenAL_HEADER_DIR}/mtsOpenALPlayQtComponent.h
                 mtsOpenALRecordQtComponent.cpp
                 ${sawOpenAL_HEADER_DIR}/mtsOpenALRecordQtComponent.h
                 sawNoteRecorderQtComponent.cpp
                 ${sawOpenAL_HEADER_DIR}/sawNoteRecorderQtComponent.h
                )

    target_link_libraries (sawOpenALQt ${QT_LIBRARIES})
    cisst_target_link_libraries (sawOpenALQt ${REQUIRED_CISST_LIBRARIES} cisstVectorQt cisstQt)
  endif (CISST_HAS_QT)


  configure_file ("${CMAKE_CURRENT_SOURCE_DIR}/sawOpenALConfig.cmake.in"
                  "${sawOpenAL_BINARY_DIR}/sawOpenALConfig.cmake"
                   @ONLY@)

else (cisst_FOUND_AS_REQUIRED)
  message ("Information: code in ${CMAKE_CURRENT_SOURCE_DIR} will not be compiled, it requires ${REQUIRED_CISST_LIBRARIES}")
endif (cisst_FOUND_AS_REQUIRED)
