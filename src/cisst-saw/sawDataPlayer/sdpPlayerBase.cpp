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


#include "sdpPlayerBase.h"
#include <cisstMultiTask/mtsInterfaceProvided.h>
#include <cisstMultiTask/mtsInterfaceRequired.h>
#include <QMessageBox>

CMN_IMPLEMENT_SERVICES(sdpPlayerBase);

sdpPlayerBase::sdpPlayerBase(const std::string & name, double period):
    mtsTaskPeriodic(name,period),
    Sync(false),
    State(STOP),
    Time(0),
    PlayStartTime(0),
    HasRunOnce(false)
{
    ErrorMessageDialog = new QErrorMessage();
    PlayerDataInfo.Name() = name;

    StateTable.AddData(Sync,       "Sync");
    StateTable.AddData(State,      "State");
    StateTable.AddData(Time,       "Time");
    StateTable.AddData(PlayStartTime,  "PlayStartTime");
    StateTable.AddData(PlayerDataInfo,  "PlayerDataInfo");
    StateTable.AddData(PlayUntilTime,  "PlayUntilTime");

    Time.SetAutomaticTimestamp(false);
    PlayStartTime.SetAutomaticTimestamp(false);
    PlayUntilTime.SetAutomaticTimestamp(false);

    mtsInterfaceProvided * provided = AddInterfaceProvided("ProvidesStatus");
    if (provided) {
        provided->AddCommandReadState(StateTable, Sync,         "IsSyncing");
        provided->AddCommandReadState(StateTable, State,        "GetState");
        provided->AddCommandReadState(StateTable, Time,         "GetTime");
        provided->AddCommandWrite(&sdpPlayerBase::SetTime, this, "WriteTime", mtsDouble());
        provided->AddCommandVoid(&sdpPlayerBase::LoadData, this, "LoadData");
        provided->AddCommandReadState(StateTable, PlayStartTime,   "GetPlayStartTime");
        provided->AddCommandReadState(StateTable, PlayerDataInfo,   "GetPlayerDataInfo");
        provided->AddCommandReadState(StateTable, PlayUntilTime,   "GetPlayUntilTime");
    }

    mtsInterfaceRequired *reqMan = AddInterfaceRequired("RequiresPlayerManager",MTS_OPTIONAL);

    if (reqMan) {
        reqMan->AddFunction("PlayRequest",        PlayRequest);
        reqMan->AddFunction("SeekRequest",        SeekRequest);
        reqMan->AddFunction("StopRequest",        StopRequest);
        reqMan->AddFunction("UpdatePlayerInfo",   UpdatePlayerInfo);
        reqMan->AddFunction("UpdateSaveParams",   UpdateSaveParams);

        reqMan->AddEventHandlerWrite(&sdpPlayerBase::PlayEventHandler,    this, "Play");
        reqMan->AddEventHandlerWrite(&sdpPlayerBase::StopEventHandler,    this, "Stop");
        reqMan->AddEventHandlerWrite(&sdpPlayerBase::SeekEventHandler,    this, "Seek");
        reqMan->AddEventHandlerWrite(&sdpPlayerBase::SaveEventHandler,    this, "Save");
        reqMan->AddEventHandlerVoid(&sdpPlayerBase::QuitEventHandler,     this, "Quit");
        reqMan->AddEventHandlerVoid(&sdpPlayerBase::ShowEventHandler,     this, "Show");


    }

    // add Requred interface which point back to our own state table
    // The reason is we may use those interfaces in the QT thread
    mtsInterfaceRequired * interfaceRequired = AddInterfaceRequired("GetStatus", MTS_OPTIONAL);
    if (interfaceRequired) {
        interfaceRequired->AddFunction("IsSyncing", BaseAccess.IsSyncing);
        interfaceRequired->AddFunction("GetState", BaseAccess.GetState);
        interfaceRequired->AddFunction("GetTime", BaseAccess.GetTime);
        interfaceRequired->AddFunction("WriteTime", BaseAccess.WriteTime);
        interfaceRequired->AddFunction("GetPlayStartTime", BaseAccess.GetPlayStartTime);
        interfaceRequired->AddFunction("GetPlayerDataInfo", BaseAccess.GetPlayerDataInfo);
        interfaceRequired->AddFunction("GetPlayUntilTime", BaseAccess.GetPlayUntilTime);
        interfaceRequired->AddFunction("LoadData", BaseAccess.LoadData);

    }

    TimeServer = mtsTaskManager::GetInstance()->GetTimeServer();


    QObject::connect(this, SIGNAL(QSignalUpdateQT()),
                     this, SLOT( QSlotUpdateQT()) );

//    QObject::connect(this, SIGNAL(QSignalShowQT()),
//                     this, SLOT( QSlotShowQT()));

}

void sdpPlayerBase::ErrorMessage(const std::string &msg){

    //    ErrorMessageDialog->showMessage(tr(msg.c_str()));

    QMessageBox::critical(this->GetWidget(), tr(GetName().c_str()),tr(msg.c_str()));
}


//! Check if timestamp is within the timestamp range of the data
bool sdpPlayerBase::IsInRange(const double &time) {

    if (time > PlayerDataInfo.DataEnd())
        return false;
    if (time < PlayerDataInfo.DataStart())
       return false;

    return true;

}

//! Adjust timestamp so it is within the range of the data
bool sdpPlayerBase::SetInRange(double &time) {

    double oldTime = time;

    if (time >  PlayerDataInfo.DataEnd())
        time =  PlayerDataInfo.DataEnd();
    if (time < PlayerDataInfo.DataStart())
        time = PlayerDataInfo.DataStart();

    if (oldTime == time)
        return true;
    else
        return false;
}

void sdpPlayerBase::SetTime(const mtsDouble &time){
    Time = time;
}


