/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*
  $Id: sdpPlayerNotes.cpp 3075 2011-10-19 02:54:23Z mbalick1 $
  
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

#include "sdpPlayerNotes.h"
#include <math.h>
#include <QMenu>
#include <QFileDialog>
#include <cisstOSAbstraction/osaGetTime.h>

#include <iostream>
#include <sstream>

CMN_IMPLEMENT_SERVICES(sdpPlayerNotes);

sdpPlayerNotes::sdpPlayerNotes(const std::string & name, double period):
    sdpPlayerBase(name,period),
    CurrentNoteID(-1)
{
    // create the user interface

    TableWidget.setRowCount(0);
    TableWidget.setColumnCount(2);

    ExWidget.setupUi(&Widget);

    QGridLayout *CentralLayout = new QGridLayout(&MainWidget);
    CentralLayout->setContentsMargins(0, 0, 0, 0);
    CentralLayout->setRowStretch(0, 1);
    CentralLayout->setColumnStretch(1, 1);

    CentralLayout->addWidget(&TableWidget, 0, 0, 1, 1);
    CentralLayout->addWidget(&Widget,1,0,1,1);


}


sdpPlayerNotes::~sdpPlayerNotes()
{
}


void sdpPlayerNotes::MakeQTConnections(void)
{
    QObject::connect(ExWidget.PlayButton, SIGNAL(clicked()),
                     this, SLOT( QSlotPlayClicked()) );

    QObject::connect(ExWidget.TimeSlider, SIGNAL(sliderMoved(int)),
                     this, SLOT( QSlotSeekSliderMoved(int)) );

    QObject::connect(ExWidget.SyncCheck, SIGNAL(clicked(bool)),
                     this, SLOT( QSlotSyncCheck(bool)) );

    QObject::connect(ExWidget.StopButton, SIGNAL(clicked()),
                     this, SLOT( QSlotStopClicked()) );

    QObject::connect(ExWidget.SetSaveStartButton, SIGNAL(clicked()),
                     this, SLOT( QSlotSetSaveStartClicked()) );

    QObject::connect(ExWidget.SetSaveEndButton, SIGNAL(clicked()),
                     this, SLOT( QSlotSetSaveEndClicked()) );

    QObject::connect(ExWidget.OpenFileButton, SIGNAL(clicked()),
                     this, SLOT( QSlotOpenFileClicked()) );

    QObject::connect(&TableWidget, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(QSlotCellDoubleClicked(int,int)) );


}

void sdpPlayerNotes::QSlotOpenFileClicked(void) {

    CMN_LOG_CLASS_RUN_VERBOSE << "Loading Notes Log : " << std::endl;

    //open file dialog.
    QString fileName = QFileDialog::getOpenFileName(&Widget, tr("Select notes text file"), tr("./"), tr("Text (*.txt)"));

    std::ifstream notesFile(fileName.toStdString().c_str());

    if (notesFile.is_open())
    {

        //remove current ones.

        TimestampsVec.clear();

        QVector<QString> noteVec;
        QVector<QString> timeVec;

        std::string line;
        while ( notesFile.good() )
        {
            getline (notesFile,line);
            QString str(QString::fromStdString(line));
            std::cout<<"Processing line:" << line<<std::endl;
            QStringList list = str.split(":");
            if (list.size() == 1) {
                if ( list[0] == str) {
                    CMN_LOG_CLASS_RUN_ERROR<< "Trouble loading note on line - "<< line<<std::endl;
                }
                else {
                    CMN_LOG_CLASS_RUN_ERROR<< "single split found"<< line<<std::endl;
                }
            }
            else {

                TimestampsVec.push_back(list[0].toDouble());
                TableWidget.setSortingEnabled(false);

                timeVec.push_back(list[0]);
                list.removeFirst();
                noteVec.push_back(list.join(":"));  //replaced the delimiter if there was one.

            }
        }

        notesFile.close();

        TableWidget.setSortingEnabled(false);
        TableWidget.setColumnCount(2);
        TableWidget.clear();
        TableWidget.setRowCount(TimestampsVec.size());
        TableWidget.setColumnCount(2);
        TableWidget.setAlternatingRowColors(true);
        TableWidget.setAutoScroll(true);
        TableWidget.setStyleSheet("QTableView {selection-background-color: green; selection-color: white;}");

        TableWidget.setEditTriggers(QAbstractItemView::NoEditTriggers);
        //TableWidget.setEditTriggers(QAbstractItemView::SelectedClicked);
        TableWidget.setSelectionBehavior(QAbstractItemView::SelectRows);
       // TableWidget.setSelectionMode(QAbstractItemView::SingleSelection);
//      But, with "ItemIsSelectable", "ItemIsEditable" and "ItemIsUserCheckable" flags the double click signal is not sent  with both cell and item signals.


        for (int row = 0; row < TimestampsVec.size(); row ++) {
            QTableWidgetItem *timeItem = new QTableWidgetItem(timeVec[row]);
            TableWidget.setItem(row, 0, timeItem);
            QTableWidgetItem *noteItem = new QTableWidgetItem(noteVec[row]);
            TableWidget.setItem(row, 1, noteItem);
        }

#if QT_VERSION >= 0x050000
        TableWidget.horizontalHeader()->sectionResizeMode(QHeaderView::ResizeToContents);
#else
        TableWidget.horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
#endif
     
        CMN_LOG_CLASS_RUN_VERBOSE<< "Loaded "<< TimestampsVec.size()<<" notes "<< fileName.toStdString()<<std::endl;
        BaseAccess.LoadData();
    }
    else {
        CMN_LOG_CLASS_RUN_ERROR<< "Unable to open file" << fileName.toStdString()<<std::endl;
        ErrorMessage("Unable to open file" + fileName.toStdString());
    }

}


void sdpPlayerNotes::Configure(const std::string & CMN_UNUSED(filename))
{
    MakeQTConnections();
    //LoadData();
    UpdateLimits();

    // Widget.setWindowTitle(QString::fromStdString(GetName()));
    // Widget.show();

    MainWidget.setWindowTitle(QString::fromStdString(GetName()));
    MainWidget.show();
}


void sdpPlayerNotes::Startup(void)
{

}

void sdpPlayerNotes::Run(void)
{
    ProcessQueuedEvents();
    ProcessQueuedCommands();

    //update the model (load data) etc.
    if (State == PLAY) {

        double currentTime = TimeServer.GetAbsoluteTimeInSeconds();
        Time = currentTime - PlayStartTime.Timestamp() + PlayStartTime.Data;

        if (Time > PlayUntilTime)  {
            Time = PlayUntilTime;
            State = STOP;
        }
        else {
            //Load and Prep current data
        }
    }
    //make sure we are at the correct seek position.
    else if (State == SEEK) {
        //Load and Prep current data
    }

    else if (State == STOP) {
        //do Nothing
    }

    //now display updated data in the qt thread space.
    if (Widget.isVisible()) {
        emit QSignalUpdateQT();
    }
}


//in QT thread space
void sdpPlayerNotes::UpdateQT(void)
{
    if (State == PLAY) {
        //Display the last datasample before Time.
        ExWidget.TimeSlider->setValue((int)Time.Data);

        if (!TimestampsVec.isEmpty() ) {

            //returns the note id of the last note before or = to the timestamp
            CurrentNoteID = FindNote(Time);
            if (CurrentNoteID != -1) {
                TableWidget.setCurrentCell(CurrentNoteID,0);
            }
        }
    }
    //Make sure we are at the correct seek location.
    else if (State == STOP) {
        //Optional: Test if the data needs to be updated:
        ExWidget.TimeSlider->setValue((int)Time.Data);

        CurrentNoteID = FindNote(Time);
        if (CurrentNoteID != -1) {
            TableWidget.setCurrentCell(CurrentNoteID,0);
        }
    }

    else if (State == SEEK) {
        //Optional: Test if the data needs to be updated:
        ExWidget.TimeSlider->setValue((int)Time.Data);
        int id = FindNote(Time);
        if (CurrentNoteID != id && id != -1) {
             CurrentNoteID = id;
             TableWidget.setCurrentCell(CurrentNoteID,0);

        }
    }
    ExWidget.TimeLabel->setText(QString::number(Time.Data,'f', 3));
}

void sdpPlayerNotes::Play(const mtsDouble & time)
{
    if (Sync) {
        CMN_LOG_CLASS_RUN_DEBUG << "Play " << PlayStartTime << std::endl;
        State = PLAY;
        PlayUntilTime = PlayerDataInfo.DataEnd();
        PlayStartTime = time;
    }
}


void sdpPlayerNotes::Stop(const mtsDouble & time)
{
    if (Sync) {
        CMN_LOG_CLASS_RUN_DEBUG << "Stop " << time << std::endl;
        PlayUntilTime = time;
    }
}


void sdpPlayerNotes::Seek(const mtsDouble & time)
{
    if (Sync) {
        CMN_LOG_CLASS_RUN_DEBUG << "Seek " << time << std::endl;

        State = SEEK;
        PlayUntilTime = PlayerDataInfo.DataEnd();
        Time = time;
    }
}


void sdpPlayerNotes::Save(const sdpSaveParameters & saveParameters)
{
    if (Sync) {
        CMN_LOG_CLASS_RUN_DEBUG << "Save " << saveParameters << std::endl;
    }
}

void sdpPlayerNotes::Quit(void)
{
    CMN_LOG_CLASS_RUN_DEBUG << "Quit" << std::endl;
    this->Kill();
}


void sdpPlayerNotes::QSlotPlayClicked(void)
{
    mtsDouble playTime = Time; //this should be read from the state table!!!
    playTime.Timestamp() = TimeServer.GetAbsoluteTimeInSeconds();

    if (Sync) {
        PlayRequest(playTime);
    } else {
        //not quite thread safe, if there is mts play call this can be corrupt.
        State = PLAY;
        PlayUntilTime = PlayerDataInfo.DataEnd();
        PlayStartTime = playTime;
    }
}


void sdpPlayerNotes::QSlotSeekSliderMoved(int c)
{
    mtsDouble t = c;

    if (Sync) {
        SeekRequest(t);
    } else {
        State = SEEK;
        Time = t;
    }
    PlayUntilTime = PlayerDataInfo.DataEnd();
}


void sdpPlayerNotes::QSlotSyncCheck(bool checked)
{
    Sync = checked;
}


void sdpPlayerNotes::QSlotStopClicked(void)
{
    mtsDouble now = Time;

    if (Sync) {
        StopRequest(now);
    } else {
        PlayUntilTime = now;
    }
}


void sdpPlayerNotes::LoadData(void)
{
    if (TimestampsVec.isEmpty()) {

        PlayerDataInfo.DataStart() = 0;
        PlayerDataInfo.DataEnd() = 0;
        CurrentNoteID = -1;
    }
    else if (TimestampsVec.size() == 1) {
        PlayerDataInfo.DataStart() = TimestampsVec[0];
        PlayerDataInfo.DataEnd() = TimestampsVec[0];
        CurrentNoteID = 0;
    }
    else {
        PlayerDataInfo.DataStart() = TimestampsVec.first();
        PlayerDataInfo.DataEnd() = TimestampsVec.last();
        CurrentNoteID = 0;
    }

    if (Time < PlayerDataInfo.DataStart()) {
        Time = PlayerDataInfo.DataStart();
    }

    if (Time > PlayerDataInfo.DataEnd()) {
        Time = PlayerDataInfo.DataEnd();
    }

    //This is the standard.
    PlayUntilTime = PlayerDataInfo.DataEnd();

    UpdatePlayerInfo(PlayerDataInfo);
    UpdateLimits();
}


void sdpPlayerNotes::QSlotSetSaveStartClicked(void)
{
    ExWidget.SaveStartSpin->setValue(Time.Data);
}

void sdpPlayerNotes::QSlotSetSaveEndClicked(void)
{
    ExWidget.SaveEndSpin->setValue(Time.Data);
}


void sdpPlayerNotes::UpdateLimits()
{
    ExWidget.TimeSlider->setRange((int)PlayerDataInfo.DataStart(), (int)PlayerDataInfo.DataEnd());

    ExWidget.TimeStartLabel->setText( QString::number(PlayerDataInfo.DataStart(),'f', 3));
    ExWidget.TimeEndLabel->setText( QString::number( PlayerDataInfo.DataEnd(),'f', 3));

    ExWidget.SaveStartSpin->setRange( PlayerDataInfo.DataStart(), PlayerDataInfo.DataEnd());
    ExWidget.SaveEndSpin->setRange( PlayerDataInfo.DataStart(), PlayerDataInfo.DataEnd());
}

void sdpPlayerNotes::SetSynced(bool isSynced) {

    ExWidget.SyncCheck->setChecked(isSynced);
    Sync = isSynced;
}



//returns the note id of the last note before or = to the timestamp
int sdpPlayerNotes::FindNote(double timestamp) {

    if (TimestampsVec.isEmpty()) {
        return -1;
    }

    if (TimestampsVec.size() == 1 ) {
        if (TimestampsVec.first() <= timestamp)
            return 0;
        else
            return -1;
    }

    //linear search for now since we don't know what the notes look like but we can assume they are in chrono order
    for (int i = 0; i < TimestampsVec.size() - 1; i++) {

        if (TimestampsVec[i] <= timestamp && TimestampsVec[i+1] >= timestamp) {

            if (TimestampsVec[i+1] == timestamp)
                  return i+1;
            else
                return i;
        }
    }

    return -1;

}

void sdpPlayerNotes::QSlotCellDoubleClicked(int row, int column) {

    CMN_LOG_CLASS_RUN_VERBOSE<< "Clicked "<< row<<" . "<<column<<std::endl;

    mtsDouble t = TimestampsVec[row];

    if (Sync) {
        SeekRequest(t);
    } else {
        State = SEEK;
        Time = t;
    }
    PlayUntilTime = PlayerDataInfo.DataEnd();
}


