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

set (REQUIRED_CISST_LIBRARIES cisstCommon cisstOSAbstraction cisstVector cisstVectorQt cisstMultiTask cisstStereoVision cisstStereoVisionQt cisstQt)

# find cisst and make sure the required libraries have been compiled
find_package (cisst REQUIRED ${REQUIRED_CISST_LIBRARIES} QUIET)

if (cisst_FOUND_AS_REQUIRED)

  # load cisst configuration
  include (${CISST_USE_FILE})

  include_directories (${PROJECT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

  # sawOpenAL has been compiled within cisst, we should find it automatically
  cisst_find_saw_component (sawOpenAL QUIET)

  if (sawOpenAL_FOUND)
    set (audioPlayer_Sources
         sdpPlayerAudio.h
         sdpPlayerAudio.cpp)

    set (audioPlayer_Header
         sdpPlayerAudio.h)

   add_definitions(-DsawPlayer_has_sawOpenAL)
  endif (sawOpenAL_FOUND)


 if (CISST_HAS_QT4)

  qt4_wrap_cpp (QT_WRAPPED_CPP
                sdpPlayerBase.h
                sdpPlayerExample.h
                sdpPlayerManager.h
                sdpPlayerVideo.h
                sdpPlayerPlot2D.h
	              sdpPlayerNotes.h
                ${audioPlayer_Header}
                )

  qt4_wrap_ui (QT_WRAPPED_UI
               sdpPlayerManagerWidget.ui
               sdpPlayerWidget.ui)

   else (CISST_HAS_QT4)
        # assumes Qt 5 Auto Moc
        set (CMAKE_AUTOMOC ON)
        qt5_wrap_cpp (QT_WRAPPED_CPP
                sdpPlayerBase.h
                sdpPlayerExample.h
                sdpPlayerManager.h
                sdpPlayerVideo.h
                sdpPlayerPlot2D.h
                sdpPlayerNotes.h
                ${audioPlayer_Header}
                )

        qt5_wrap_ui (QT_WRAPPED_UI
               sdpPlayerManagerWidget.ui
               sdpPlayerWidget.ui)

  endif (CISST_HAS_QT4)
        
  add_library (sawDataPlayerComponents ${IS_SHARED}
               ${QT_WRAPPED_CPP}
               ${QT_WRAPPED_UI}
               sdpExport.h
               sdpPlayerBase.h
               sdpPlayerBase.cpp
               sdpPlayerManager.h
               sdpPlayerManager.cpp
               sdpSaveParameters.h
               sdpSaveParameters.cpp
               sdpPlayerDataInfo.h
               sdpPlayerDataInfo.cpp
               sdpPlayerExample.h
               sdpPlayerExample.cpp
               sdpPlayerPlot2D.h
               sdpPlayerPlot2D.cpp
               sdpPlayerVideo.h
               sdpPlayerVideo.cpp
               sdpPlayerParseStateTableData.h
               sdpPlayerParseStateTableData.cpp
               sdpPlayerNotes.h
               sdpPlayerNotes.cpp
               ${audioPlayer_Sources}
               )
  set_property (TARGET sawDataPlayerComponents PROPERTY FOLDER "sawDataPlayer")
  cisst_target_link_libraries (sawDataPlayerComponents ${REQUIRED_CISST_LIBRARIES})

  if (sawOpenAL_FOUND)

    # sawOpenAL configuration
    include_directories (${sawOpenAL_INCLUDE_DIR})
    link_directories (${sawOpenAL_LIBRARY_DIR})
    target_link_libraries (sawDataPlayerComponents ${sawOpenAL_LIBRARIES})

  endif (sawOpenAL_FOUND)

  add_executable (sawDataPlayer
                  main.cpp)

  target_link_libraries (sawDataPlayer sawDataPlayerComponents)
  set_property (TARGET sawDataPlayer PROPERTY FOLDER "sawDataPlayer")
  cisst_target_link_libraries (sawDataPlayer ${REQUIRED_CISST_LIBRARIES})

  add_executable (sawQtVideoPlayer
                  qtVideoPlayer.cpp)
  set_property (TARGET sawQtVideoPlayer PROPERTY FOLDER "sawDataPlayer")
  target_link_libraries (sawQtVideoPlayer sawDataPlayerComponents)
  cisst_target_link_libraries (sawQtVideoPlayer ${REQUIRED_CISST_LIBRARIES})

else (cisst_FOUND_AS_REQUIRED)
  cisst_information_message_missing_libraries (${REQUIRED_CISST_LIBRARIES})
endif (cisst_FOUND_AS_REQUIRED)
