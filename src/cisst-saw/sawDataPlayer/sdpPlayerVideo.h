/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*

  Author(s): Marcin Balicki
  Created on: 2011-02-10

  (C) Copyright 2011 Johns Hopkins University (JHU), All Rights
  Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---
*/

#ifndef _sdpPlayerVideo_h
#define _sdpPlayerVideo_h

#include <QObject>
#include <QtGui/QCloseEvent>
#include <QPushButton>
#include <QCheckBox>
#include <QSpinBox>
#include "ui_sdpPlayerWidget.h"

#include "sdpPlayerBase.h"

#include <cisstStereoVision/svlStreamManager.h>
#include <cisstStereoVision/svlFilterImageOverlay.h>
#include <cisstStereoVision/svlFilterImageOpenGLQtWidget.h>
#include <cisstStereoVision/svlFilterSourceVideoFile.h>
#include <cisstStereoVision/svlFilterVideoFileWriter.h>
#include <cisstStereoVision/svlFilterImageCropper.h>

//! todo Think about the thread safety issue. (At the moment, mtscommands and qslots can manipulte same data at the same time)
//! If the data processing is not heavy we can use the main QT thread via QTimer and mtsTaskFromCallback, or add more mtsFunction calls.

// Always include last
#include "sdpExport.h"

class CISST_EXPORT sdpPlayerVideo: public sdpPlayerBase
{
    Q_OBJECT;
    CMN_DECLARE_SERVICES(CMN_NO_DYNAMIC_CREATION, CMN_LOG_LOD_RUN_DEBUG);

public:
    sdpPlayerVideo(const std::string & name, double period);
    ~sdpPlayerVideo();

    //call this after tasks connections are made
    void Configure(const std::string & filename = "");

    void Startup(void);
    void Run(void);
    void Cleanup(void) {};

    //If not synced then the player needs to go through Manager in order to play.
    void SetSynced(bool isSynced);

protected:
    bool eventFilter( QObject *dist, QEvent *event );


private:

    Ui::sdpPlayerWidget ExWidget;
    void MakeQTConnections(void);
    //by calling "emit QSignalQTUpdate" this function will be called.
    //used this to udpate qt widgets in a thread safe way.
    void UpdateQT(void);

    void Stop(const mtsDouble & time);
    void Seek(const mtsDouble & time);
    void Play(const mtsDouble & time);
    void Save(const sdpSaveParameters & saveParameters);
    void Quit(void);
    void LoadData(void);
    void UpdateLimits(void);

    void SetupPipeline();

    void SeekForwardOne();
    void SeekReverseOne();

    QWidget MainWindow;
    svlFilterImageOpenGLQtWidget * VideoWidget;

     // instantiating SVL stream and filters
    svlStreamManager            StreamManager;
    svlFilterImageOverlay       Overlay;
    svlFilterSourceVideoFile    Source;
    svlFilterImageCropper       Cropper;

    svlOverlayTimestamp         * TimestampOverlay;

    std::string                 FileName;
    QCheckBox                   *CropButton;
    QCheckBox                   *SaveAsBMPButton;
    QCheckBox                   *SwapRGB_Button;
    QSpinBox                    *LeftSpinBox;
    QSpinBox                    *RightSpinBox;
    QSpinBox                    *TopSpinBox;
    QSpinBox                    *BottomSpinBox;

    svlRect                     CropRect;
    int                         VideoWidth;
    int                         VideoHeight;


private slots:

    void QSlotSeekSliderMoved(int c);
    void QSlotSyncCheck(bool checked);
    void QSlotPlayClicked(void);
    void QSlotStopClicked(void);
    void QSlotSetSaveStartClicked(void);
    void QSlotSetSaveEndClicked(void);
    void QSlotOpenFileClicked(void);
    void QSlotSetRangeClicked(void);
    void QSlotCropButtonClicked(bool checked);

};

CMN_DECLARE_SERVICES_INSTANTIATION(sdpPlayerVideo);

#endif // _sdpPlayerVideo_h
