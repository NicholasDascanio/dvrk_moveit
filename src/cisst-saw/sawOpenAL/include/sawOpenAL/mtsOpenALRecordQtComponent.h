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

#ifndef _mtsOpenALRecordQtComponent_h
#define _mtsOpenALRecordQtComponent_h

#include <QObject>
#include <QDockWidget>
#include <QTimerEvent>
#include <QErrorMessage>

#include <cisstVector/vctPlot2DOpenGLQtWidget.h>
#include <cisstOSAbstraction/osaTimeServer.h>
#include <cisstMultiTask/mtsComponent.h>

#include "ui_mtsOpenALRecordQtWidget.h"

// Always include last!
#include "sawOpenALExportQt.h"

//! /todo How to manage start and stop of the
//! PlayerManager should show the start and end of the data??

//! /todo Decide how to deal with path/prefix
//! /todo Centralize quit button.


class CISST_EXPORT mtsOpenALRecordQtComponent: public QObject, public mtsComponent
{
    Q_OBJECT;
    CMN_DECLARE_SERVICES(CMN_NO_DYNAMIC_CREATION, CMN_LOG_ALLOW_DEFAULT);

 public:

    QDockWidget * GetWidget(void) {
        return &Widget;
    }

    //
    mtsOpenALRecordQtComponent(const std::string & name, double updatePeriod);
    ~mtsOpenALRecordQtComponent() {};

    void Configure(const std::string & filename = "");

    //States of operation
    enum {STOP, PLAY, SEEK};

 private:
    //The instance of the widget representing this behavior.
    QDockWidget         Widget;

    Ui::mtsOpenALRecordQtWidget RecWidget;
    void MakeQTConnections(void);

    mtsInt  State;

    //Current time (aka position) in the data playback process
    mtsDouble Time;

    //the time when current play was started, timestamp is the cpu clock,
    //the .Data is the corresponding time in the Data stream
    mtsDouble PlayStartTime;


    struct {
        mtsFunctionRead     GetIsRecording;
        mtsFunctionWrite    SetFileName;
        mtsFunctionVoid     Start;
        mtsFunctionVoid     Stop;
        mtsFunctionWrite    SetCaptureDeviceName;
        mtsFunctionWrite    SetCaptureDeviceID;
        mtsFunctionRead     GetCaptureDeviceNames;
        mtsFunctionRead     GetStreamVolume;
        mtsFunctionRead     GetTime;                //elapsed time.
        mtsFunctionRead     GetFileSize;
    } Recorder;


    osaTimeServer TimeServer;
    QErrorMessage * ErrorMessageDialog;
    void ErrorMessage(const std::string & message);

    void timerEvent(QTimerEvent *event);

    vctPlot2DOpenGLQtWidget     * Plot;
    vctPlot2DBase::Signal       * DataTrace;

 public slots:

    void QSlotRecordClicked();
    void QSlotStopClicked();
    void QSlotPathClicked(void);
    void QSlotDeviceSelected(const QString & str);

 signals:


};

CMN_DECLARE_SERVICES_INSTANTIATION(mtsOpenALRecordQtComponent)

#endif  //_mtsOpenALRecordQtComponent_h
