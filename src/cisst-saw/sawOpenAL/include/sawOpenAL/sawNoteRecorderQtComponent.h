/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*
  $Id: sawNoteRecorderQtComponent.h 3361 2012-01-19 14:24:13Z mbalick1 $

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

#ifndef _sawNoteRecorderQtComponent_h
#define _sawNoteRecorderQtComponent_h

#include <QObject>
#include <QDockWidget>
#include <QTimerEvent>
#include <QErrorMessage>
#include <QSlider>

#include <cisstVector/vctPlot2DOpenGLQtWidget.h>
#include <cisstOSAbstraction/osaTimeServer.h>
#include <cisstMultiTask/mtsComponent.h>
#include <cisstMultiTask/mtsManagerLocal.h>

#include "ui_sawNoteRecorderQtWidget.h"
#include <QButtonGroup>
#include <QMap>
#include <QPushButton>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDockWidget>
#include <QSizePolicy>

#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <fstream>


// Always include last!
#include <sawOpenAL/sawOpenALExportQt.h>


//! /todo Decide how to deal with path/prefix
//! /todo Centralize quit button.

class NoteWidget : public QWidget {
    Q_OBJECT

public:
    //needs to register
    NoteWidget( const QObject * noteRecorder, QString note) :
        QWidget(),
        SaveButton("Note"),
        RemoveButton("X"),
        ClickTime(0){

        RemoveButton.setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        SaveButton.setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
        NoteLineEdit.setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

        QHBoxLayout *horLayout = new QHBoxLayout(this) ;

        this->setLayout(horLayout);

        horLayout->addWidget(&SaveButton);
        horLayout->addWidget(&NoteLineEdit);
        horLayout->addWidget(&RemoveButton);

        NoteLineEdit.setText(note);
        connect(this, SIGNAL(QSignalSaveClicked(NoteWidget *)), noteRecorder, SLOT(QSlotLogNoteClicked(NoteWidget *)));
        connect(this, SIGNAL(QSignalRemoveClicked(NoteWidget *)), noteRecorder, SLOT(QSlotRemoveNoteClicked(NoteWidget *)));

        connect(&SaveButton,  SIGNAL(clicked()), this, SLOT(QSlotSaveClicked()));
        connect(&RemoveButton, SIGNAL(clicked()), this, SLOT(QSlotRemoveClicked()));
        connect(&NoteLineEdit, SIGNAL( returnPressed()), this, SLOT(QSlotSaveClicked()));
        RemoveButton.setToolTip("Remove Note from GUI");

        RemoveButton.setStyleSheet(" QPushButton { background: qradialgradient(cx:0, cy:0, radius: 2, fx:0.5, fy:0.5, stop:0 white, stop:1 rgba(190,0,0, 60%)); border-radius: 3px;}");
        //SaveButton.setStyleSheet(" QPushButton { background: qradialgradient(cx:0, cy:0, radius: 2, fx:0.5, fy:0.5, stop:0 white, stop:1 rgba(0,210,0, 60%)); border-radius: 10px;}");
        SaveButton.setStyleSheet(" QPushButton { background-color :  rgb(0, 180, 0); }");

    }

    void UpdateGUI(double currentTime) {
        double t = currentTime - ClickTime;
        double decayTime = 1;
        if (t <= decayTime) {

           int color = static_cast<int>(255 - (t/decayTime * 255));

           QString ss = "QLineEdit { background-color :  hsv(120," + QString::number(color) + ",255); }";
           NoteLineEdit.setStyleSheet(ss);
     //        NoteLineEdit.setStyleSheet(" QLineEdit { background: qradialgradient(cx:0, cy:0, radius: 1, fx:0.5, fy:0.5, stop:0 white, stop:1 rgba(0,190,0, 60%)); border-radius: 9px;}");
        }
    }

    QString GetText(void) {return NoteLineEdit.text();};

    ~NoteWidget() {};

private :
    NoteWidget();

    QPushButton SaveButton;
    QPushButton RemoveButton;
    QLineEdit   NoteLineEdit;

    double ClickTime;


public slots:

    void QSlotRemoveClicked() {
        emit QSignalRemoveClicked(this);
    }

    void QSlotSaveClicked(){
        ClickTime = mtsManagerLocal::GetInstance()->GetTimeServer().GetAbsoluteTimeInSeconds();
        emit QSignalSaveClicked(this);
    }

signals:
    void QSignalSaveClicked(NoteWidget *);
    void QSignalRemoveClicked(NoteWidget *);

};

class CISST_EXPORT sawNoteRecorderQtComponent: public QObject, public mtsComponent
{
    Q_OBJECT;
    CMN_DECLARE_SERVICES(CMN_NO_DYNAMIC_CREATION, CMN_LOG_ALLOW_VERBOSE);

public:

    QDockWidget * GetWidget(void) {
        return &Widget;
    }

    sawNoteRecorderQtComponent(const std::string & name, double updatePeriod);
    ~sawNoteRecorderQtComponent();
    void Configure(const std::string & filename = "");

    void Show() { Widget.show(); }
    void Hide() { Widget.hide(); }

    void LogNote(const mtsStdString &note);

private:
    //The instance of the widget representing this behavior.
    QDockWidget         Widget;
    QVBoxLayout         NotesVLayout;

    void AddNote(const QString & noteTxt);

    //Store and Load notes from files.
    void LoadPresets();
    void SavePresets();

    Ui::sawNoteRecorderQtWidget NoteRecorderWidget;
    void MakeQTConnections(void);

    QVector<NoteWidget *> NoteVec;
    std::ofstream   LogFile;

    osaTimeServer TimeServer;
    QErrorMessage * ErrorMessageDialog;
    void ErrorMessage(const std::string & message);

    void timerEvent(QTimerEvent *);

    void RangeChangedEvent(void) {
        emit QSignalUpdateRange();
    }

public slots:

    void QSlotRemoveNoteClicked(NoteWidget *);
    void QSlotLogNoteClicked(NoteWidget *);

    void QSlotAddExtraNoteClicked();
    void QSlotNewLogFileClicked();

    void QSlotSavePresetsClicked() {
        SavePresets();
    }
    void QSlotLoadPresetsClicked() {
        LoadPresets();
    }

    void QSlotPathClicked(void);

signals:
    void QSignalUpdateRange(void);

};

CMN_DECLARE_SERVICES_INSTANTIATION(sawNoteRecorderQtComponent)

#endif  //_sawNoteRecorderQtComponent_h
