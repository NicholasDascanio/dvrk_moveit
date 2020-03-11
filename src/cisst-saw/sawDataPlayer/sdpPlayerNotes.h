/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*
  $Id: sdpPlayerNotes.h 3097 2011-10-22 02:32:57Z mbalick1 $

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

#ifndef _sdpPlayerNotes_h
#define _sdpPlayerNotes_h

#include <QObject>
#include <QtGui/QCloseEvent>

#include "ui_sdpPlayerWidget.h"
#include "sdpPlayerBase.h"
#include <QTableWidget>
#include <QVector>

//! todo Think about the thread safety issue. (At the moment, mtscommands and qslots can manipulte same data at the same time)
//! If the data processing is not heavy we can use the main QT thread via QTimer and mtsTaskFromCallback, or add more mtsFunction calls.

// Always include last
#include "sdpExport.h"

class CISST_EXPORT sdpPlayerNotes: public sdpPlayerBase
{
    Q_OBJECT;
    CMN_DECLARE_SERVICES(CMN_NO_DYNAMIC_CREATION, CMN_LOG_ALLOW_VERBOSE);

public:
    sdpPlayerNotes(const std::string & name, double period);
    ~sdpPlayerNotes();

    //call this after tasks connections are made
    void Configure(const std::string & filename = "");
    void Startup(void);
    void Run(void);
    void Cleanup(void) {};
    //If not synced then the player needs to go through Manager in order to play.
    void SetSynced(bool isSynced);

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

    QTableWidget         TableWidget;
    QVector<double>      TimestampsVec;
    QWidget              MainWidget;
    int                  CurrentNoteID;

    //returns the note id of the last note before or = to the timestamp
    int FindNote(double timestamp);

private slots:
    //call only mtsfunctions from these, don't access data directly.
    void QSlotSeekSliderMoved(int c);
    void QSlotSyncCheck(bool checked);
    void QSlotPlayClicked(void);
    void QSlotStopClicked(void);
    void QSlotSetSaveStartClicked(void);
    void QSlotSetSaveEndClicked(void);
    void QSlotOpenFileClicked(void);
    void QSlotCellDoubleClicked(int row, int column);

};

CMN_DECLARE_SERVICES_INSTANTIATION(sdpPlayerNotes);

#endif // _sdpPlayerNotes_h
