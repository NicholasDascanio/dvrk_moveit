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

#include "sdpPlayerManager.h"
#include <cisstMultiTask/mtsInterfaceProvided.h>
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QStringList>
#include <QList>
#include <QFileDialog>

CMN_IMPLEMENT_SERVICES(sdpPlayerManager);

sdpPlayerManager::sdpPlayerManager(const std::string & name, double period):
    mtsTaskPeriodic(name,period),
    Time(0),
    DataStartTime(0),
    DataEndTime(0),
    PlayStartTime(0),
    TimeServer(0)
{
    SaveParameters.Start() = 0;
    SaveParameters.End() = 0;
    SaveParameters.Path() = "./";
    SaveParameters.Prefix() = "";

    ErrorMessageDialog = new QErrorMessage();

    StateTable.AddData(State,           "State");
    StateTable.AddData(Time,           "Time");
    StateTable.AddData(PlayStartTime,  "PlayStartTime");
    StateTable.AddData(DataStartTime,  "DataStartTime");
    StateTable.AddData(DataEndTime,    "DataEndTime");
    StateTable.AddData(PlayUntilTime,  "PlayUntilTime");

    Time.SetAutomaticTimestamp(false);
    PlayStartTime.SetAutomaticTimestamp(false);
    DataStartTime.SetAutomaticTimestamp(false);
    DataEndTime.SetAutomaticTimestamp(false);
    PlayUntilTime.SetAutomaticTimestamp(false);

    mtsInterfaceProvided * provided = AddInterfaceProvided("ProvidesPlayerManager");
    if (provided) {
        provided->AddCommandReadState(StateTable, State,           "GetState");
        provided->AddCommandReadState(StateTable, Time,            "GetTime");
        provided->AddCommandReadState(StateTable, PlayStartTime,   "GetPlayStartTime");
        provided->AddCommandReadState(StateTable, DataStartTime,   "GetDataStartTime");
        provided->AddCommandReadState(StateTable, DataEndTime,     "GetDataEndTime");
        provided->AddCommandReadState(StateTable, PlayUntilTime,   "GetPlayUntilTime");

        provided->AddCommandWrite(&sdpPlayerManager::PlayRequestHandler,   this, "PlayRequest", mtsDouble());
        provided->AddCommandWrite(&sdpPlayerManager::SeekRequestHandler,   this, "SeekRequest", mtsDouble());
        provided->AddCommandWrite(&sdpPlayerManager::StopRequestHandler,   this, "StopRequest", mtsDouble());
        provided->AddCommandWrite(&sdpPlayerManager::UpdatePlayerInfoHandler,   this, "UpdatePlayerInfo", sdpPlayerDataInfo());
        provided->AddCommandWrite(&sdpPlayerManager::UpdateSaveParamsHandler,   this, "UpdateSaveParams", sdpSaveParameters());

        provided->AddCommandVoid(&sdpPlayerManager::QuitRequestHandler,   this, "QuitRequest");

        provided->AddEventWrite(Play, "Play", mtsDouble());
        provided->AddEventWrite(Stop, "Stop", mtsDouble());
        provided->AddEventWrite(Seek, "Seek", mtsDouble());
        provided->AddEventWrite(Save, "Save", sdpSaveParameters());
        provided->AddEventVoid(ShowAll, "Show");

        provided->AddEventVoid(Quit, "Quit");

    }

    // create the user interface
    MgrWidget.setupUi(&Widget);

    QStringList list = (QStringList() << tr("Player") << tr("Start") << tr("End") );
    MgrWidget.PlayerTable->setHorizontalHeaderLabels(list);

    TimeServer = &(mtsTaskManager::GetInstance()->GetTimeServer());
}


void sdpPlayerManager::MakeQTConnections(void)
{
    QObject::connect(MgrWidget.PlayButton, SIGNAL(clicked()),
                     this, SLOT( QSlotPlayClicked()) );

    QObject::connect(MgrWidget.TimeSlider, SIGNAL(sliderMoved(int)),
                     this, SLOT( QSlotSeekSliderMoved(int)) );

    QObject::connect(MgrWidget.StopButton, SIGNAL(clicked()),
                     this, SLOT( QSlotStopClicked()) );

    QObject::connect(MgrWidget.SetSaveStartButton, SIGNAL(clicked()),
                     this, SLOT( QSlotSetSaveStartClicked()) );

    QObject::connect(MgrWidget.SetSaveEndButton, SIGNAL(clicked()),
                     this, SLOT( QSlotSetSaveEndClicked()) );

    QObject::connect(MgrWidget.SaveButton, SIGNAL(clicked()),
                     this, SLOT( QSlotSaveClicked()) );

    QObject::connect(MgrWidget.QuitButton, SIGNAL(clicked(void)),
                     this, SLOT( QSlotQuitClicked(void)) );

    QObject::connect(MgrWidget.PathButton, SIGNAL(clicked(void)),
                     this, SLOT( QSlotPathClicked(void)) );

    QObject::connect(MgrWidget.QuitButton, SIGNAL(clicked()), qApp, SLOT(quit()));

    QObject::connect(this, SIGNAL(QSignalUpdateQT()),
                     this, SLOT( QSlotUpdateQT()) );

    QObject::connect(MgrWidget.ShowAllButton, SIGNAL(clicked()),
                     this, SLOT( QSlotShowAll()) );

}


void sdpPlayerManager::UpdateQT(void)
{
    if (State == PLAY) {
        //Display the last datasample before Time.
        MgrWidget.TimeSlider->setValue(Time.Data);
        // MgrWidget.TimeSlider->setToolTip( QString::number(Time.Data,'f', 3));
    }
    //Make sure we are at the correct seek location.
    else if (State == STOP) {
        //Optional: Test if the data needs to be updated:
        MgrWidget.TimeSlider->setValue(Time.Data);
    }

    else if (State == SEEK) {
        //Optional: Test if the data needs to be updated:
        MgrWidget.TimeSlider->setValue(Time.Data);
    }
    MgrWidget.TimeLabel->setText( QString::number(Time.Data,'f', 3));
}


void sdpPlayerManager::Configure(const std::string & CMN_UNUSED(filename))
{
    MakeQTConnections();
    //! /todo Temporary hack, remove
    // DataEndTime     = 300;
    UpdateLimits();
    PlayUntilTime = DataStartTime;

    //Widget.resize(700,300);
    Widget.show();
}


void sdpPlayerManager::Startup(void)
{

}


void sdpPlayerManager::Run(void)
{
    ProcessQueuedEvents();
    ProcessQueuedCommands();

    //update the model (load data) etc.
    if (!IsStarted()) {
        return;
    }

    if (State == PLAY) {
        double currentTime = TimeServer->GetAbsoluteTimeInSeconds();
        Time = currentTime - PlayStartTime.Timestamp() + PlayStartTime.Data;
        if (Time > PlayUntilTime) { //stop
            Time = PlayUntilTime;
            StopRequestHandler(PlayUntilTime);  //call all the players to stop
            State = STOP;
        }
    }

    //make sure we are at the correct seek position.
    else if (State == SEEK) {

        //Load and Prep current data
    }

    else if (State == STOP) {
        //do Nothing

    }

    if (Widget.isVisible()) {
        emit QSignalUpdateQT();  //now draw updated data in the qt thread space.
    }
}


void sdpPlayerManager::ErrorMessage(const std::string & message)
{
    //    ErrorMessageDialog->showMessage(tr(msg.c_str()));
    QMessageBox::critical(this->GetWidget(), tr(GetName().c_str()),tr(message.c_str()));
}


void sdpPlayerManager::StopRequestHandler(const mtsDouble & time)
{
    PlayUntilTime = time;
    Stop(time);
    CMN_LOG_CLASS_RUN_DEBUG << "StopRequest for " << time <<std::endl;

}


void sdpPlayerManager::SeekRequestHandler(const mtsDouble & time)
{
    State = SEEK;
    Time = time;
    Seek(time);
    CMN_LOG_CLASS_RUN_DEBUG << "SeekRequest for " << time <<std::endl;
}


void sdpPlayerManager::PlayRequestHandler(const mtsDouble & time)
{
    CMN_LOG_CLASS_RUN_DEBUG << "PlayRequest for " << time <<std::endl;
    State = PLAY;
    PlayStartTime = time;
    PlayUntilTime = DataEndTime;
    Play(PlayStartTime);
}


void sdpPlayerManager::UpdateSaveParamsHandler(const sdpSaveParameters & saveParameters)
{
    SaveParameters = saveParameters;
    //! @todo shall the path be updated also?  - this is not a thread safe way to update the params...but unlikely to be a problem
    //SaveParameters.Path() = MgrWidget.PathLineEdit->text().toStdString();
    //SaveParameters.Prefix() = MgrWidget.PrefixLineEdit->text().toStdString();
    MgrWidget.SaveStartSpin->setValue(SaveParameters.Start());
    MgrWidget.SaveEndSpin->setValue(SaveParameters.End());

    CMN_LOG_CLASS_RUN_DEBUG << "UpdateSaveParams received: " << saveParameters << std::endl;

}


void sdpPlayerManager::UpdatePlayerInfoHandler(const sdpPlayerDataInfo & info)
{
    bool foundPlayer = false;
    //find player
    for (size_t i = 0; i < PlayerList.size(); i++) {
        //if found just update it
        if (PlayerList[i].Name().compare(info.Name()) == 0) {
            PlayerList[i] = info;
            foundPlayer = true;
        }
    }

    if (!foundPlayer) {
        //add
        PlayerList.push_back(info);
    }

    MgrWidget.PlayerTable->clearContents();

    //    QList<QTableWidgetItem*> list;
    //    list = MgrWidget.PlayerTable->findItems(QString::fromStdString(info.Name()), Qt::MatchFixedString);

    MgrWidget.PlayerTable->setRowCount(PlayerList.size()); // add one row

    for (size_t i = 0; i < PlayerList.size(); i++) {
        QTableWidgetItem * player = new QTableWidgetItem(QString::fromStdString(PlayerList[i].Name()));
        QTableWidgetItem * start = new QTableWidgetItem(QString::number(PlayerList[i].DataStart(),'f', 3));
        QTableWidgetItem * end = new QTableWidgetItem(QString::number(PlayerList[i].DataEnd(),'f', 3));
        MgrWidget.PlayerTable->setSortingEnabled (false);
        MgrWidget.PlayerTable->setItem( i, 0, player );
        MgrWidget.PlayerTable->setItem( i, 1, start );
        MgrWidget.PlayerTable->setItem( i, 2, end );
        //! /todo : this causes connect errors (qt:sortOrder)
        //MgrWidget.PlayerTable->setSortingEnabled (true);

    }
    //    else if(list.size() > 1)
    //    {
    //        CMN_LOG_CLASS_RUN_ERROR<<" *** Conflicting names in the table "<<std::endl;

    //    }
    //    else {

    //        MgrWidget.PlayerTable->setSortingEnabled (false);
    //        MgrWidget.PlayerTable->item( list[0]->row(), 1)->setText(QString::number(info.DataStart(),'f', 3));
    //        MgrWidget.PlayerTable->item( list[0]->row(), 2)->setText(QString::number(info.DataEnd(),'f', 3));
    //        MgrWidget.PlayerTable->setSortingEnabled (true);
    //    }

    //update

    UpdateLimits();

    CMN_LOG_CLASS_RUN_DEBUG << " UpdatePlayerInfo received: " << info << std::endl;
}


void sdpPlayerManager::QuitRequestHandler(void)
{
    CMN_LOG_CLASS_RUN_DEBUG << "QuitRequest received" << std::endl;
    Quit();
    this->Kill();
}


void sdpPlayerManager::QSlotPlayClicked()
{
    mtsDouble playTime = Time; //this should be read from the state table!!!
    playTime.Timestamp() = TimeServer->GetAbsoluteTimeInSeconds();
    PlayRequestHandler(playTime);
}


void sdpPlayerManager::QSlotStopClicked()
{
    StopRequestHandler(Time);
}


void sdpPlayerManager::QSlotSaveClicked()
{
    SaveParameters.Path() = MgrWidget.PathLineEdit->text().toStdString();
    SaveParameters.Prefix() = MgrWidget.PrefixLineEdit->text().toStdString();
    SaveParameters.Start() = MgrWidget.SaveStartSpin->value();
    SaveParameters.End() = MgrWidget.SaveEndSpin->value();
    Save(SaveParameters);
    CMN_LOG_RUN_VERBOSE << "Save called with " << SaveParameters <<std::endl;
}


void sdpPlayerManager::QSlotSeekSliderMoved(int c)
{
    mtsDouble t = c;
    PlayUntilTime = DataEndTime;
    SeekRequestHandler(t);
}


void sdpPlayerManager::QSlotSetSaveStartClicked(void)
{
    MgrWidget.SaveStartSpin->setValue(Time.Data);
}


void sdpPlayerManager::QSlotSetSaveEndClicked(void)
{
    MgrWidget.SaveEndSpin->setValue(Time.Data);
}


void sdpPlayerManager::QSlotPathClicked(void)
{
    QString pathName = QFileDialog::getExistingDirectory(&Widget, tr("Select Path"),tr("./"));
    MgrWidget.PathLineEdit->setText(pathName);
}


void sdpPlayerManager::UpdateLimits(void)
{
    double end = 0 ;
    double start = 0;

    if (PlayerList.size() != 0) {
        end = PlayerList[0].DataEnd();
        start = PlayerList[0].DataStart();
        for (size_t i = 1; i < PlayerList.size(); i++) {
            if (end < PlayerList[i].DataEnd())
                end = PlayerList[i].DataEnd();
            if ( start > PlayerList[i].DataStart())
                start = PlayerList[i].DataStart();
        }
    }

    DataStartTime = start;
    DataEndTime = end;

    MgrWidget.TimeSlider->setRange((int)DataStartTime.Data, (int)DataEndTime.Data);

    MgrWidget.TimeStartLabel->setText( QString::number(DataStartTime.Data,'f', 3));
    MgrWidget.TimeEndLabel->setText( QString::number(DataEndTime.Data,'f', 3));

    MgrWidget.SaveStartSpin->setRange(DataStartTime.Data,DataEndTime.Data);
    MgrWidget.SaveEndSpin->setRange(DataStartTime.Data,DataEndTime.Data);

    if (Time < DataStartTime) {
        Time = DataStartTime;
    }

    if (Time > DataEndTime) {
        Time = DataEndTime;
    }

}


void sdpPlayerManager::AddPlayer(sdpPlayerBase *player)
{
    // connect the tasks, task.RequiresInterface -> task.ProvidesInterface
    mtsTaskManager::GetInstance()->Connect(player->GetName(), "RequiresPlayerManager",
                                           this->GetName(), "ProvidesPlayerManager");

    player->SetSynced(true);

}
