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

#ifndef _mtsOpenALPlayQtComponent_h
#define _mtsOpenALPlayQtComponent_h

#include <QObject>
#include <QDockWidget>
#include <QTimerEvent>
#include <QErrorMessage>
#include <QSlider>

#include <cisstVector/vctPlot2DOpenGLQtWidget.h>
#include <cisstOSAbstraction/osaTimeServer.h>
#include <cisstMultiTask/mtsComponent.h>

#include "ui_mtsOpenALPlayQtWidget.h"

// Always include last!
#include <sawOpenAL/sawOpenALExportQt.h>

//! /todo How to manage start and stop of the
//! PlayerManager should show the start and end of the data??

//! /todo Decide how to deal with path/prefix
//! /todo Centralize quit button.

class CISST_EXPORT mtsOpenALPlayQtComponent: public QObject, public mtsComponent
{
    Q_OBJECT;
    CMN_DECLARE_SERVICES(CMN_NO_DYNAMIC_CREATION, CMN_LOG_ALLOW_DEFAULT);

 public:

    QDockWidget * GetWidget(void) {
        return &Widget;
    }

    //
    mtsOpenALPlayQtComponent(const std::string & name, double updatePeriod);
    ~mtsOpenALPlayQtComponent() {};
    void Configure(const std::string & filename = "");
    //States of operation
    enum {STOP, PLAY, SEEK};

 private:
    //The instance of the widget representing this behavior.
    QDockWidget         Widget;
    QSlider             *SeekSlider;


    Ui::mtsOpenALPlayQtWidget PlayWidget;
    void MakeQTConnections(void);

    mtsInt  State;

    //Current time (aka position) in the data playback process
    mtsDouble Time;

    //the start and end timestamps of the data in our system. min/max.
    mtsDouble DataStartTime;
    mtsDouble DataEndTime;
    //the time when current play was started, timestamp is the cpu clock,
    //the .Data is the corresponding time in the Data stream
    mtsDouble PlayStartTime;
    //the Data stream time when the stream playback should stop
    mtsDouble PlayUntilTime;

    vctPlot2DOpenGLQtWidget     * Plot;
    vctPlot2DBase::Signal       * DataTrace;

    osaTimeServer TimeServer;
    QErrorMessage * ErrorMessageDialog;
    void ErrorMessage(const std::string & message);

    void timerEvent(QTimerEvent *);

    struct {
        mtsFunctionRead     GetIsPlaying;
        mtsFunctionVoid     Play;
        mtsFunctionVoid     Pause;
        mtsFunctionWrite    Seek;
        mtsFunctionRead     GetStartTime;
        mtsFunctionWrite    OpenFile;
        mtsFunctionRead     GetVolume;
        mtsFunctionWrite    SetVolume;
        mtsFunctionRead     GetTime;
        mtsFunctionRead     GetLengthInSec;
        mtsFunctionRead     GetStreamVolume;
    } Player;

    void RangeChangedEvent(void) {
        emit QSignalUpdateRange();
    }

 public slots:

    void QSlotPlayClicked();
    void QSlotPauseClicked();
    void QSlotFileDialogClicked();
    void QSlotOpenFileClicked();
    void QSlotVolumeSliderMoved(int v);
    void QSlotSeekSliderMoved(int v);
    void QSlotUpdateRange(void);

 signals:
    void QSignalUpdateRange(void);

};

CMN_DECLARE_SERVICES_INSTANTIATION(mtsOpenALPlayQtComponent)

#endif  //_mtsOpenALPlayQtComponent_h
