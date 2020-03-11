/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*

  Author(s):  Marcin Balicki
  Created on: 2011

  (C) Copyright 2011-2012 Johns Hopkins University (JHU), All Rights
  Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---

*/

#include <cisstOSAbstraction/osaSleep.h>
#include <cisstMultiTask/mtsTaskManager.h>
#include <sawOpenAL/sawOpenAL.h>
#include <sawOpenAL/sawOpenALQt.h>
#include <QApplication>
#include <QCoreApplication>
#include <sawOpenAL/sawNoteRecorderQtComponent.h>


//includes for kill quit signals.

#ifndef _MSC_VER  //not available on windows.

#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#endif

#ifndef _MSC_VER  //not available on windows.
void SIGQUIT_Handler(int sig)
{
    signal(sig, SIG_IGN);
    printf("From SIGQUIT: just got a %d (SIGQUIT ^\\) signal"
           " and is about to quit\n", sig);

    osaSleep(5);
    QCoreApplication::exit();

    printf("Just called Quit\n");
}

void Register_SIG_QUIT_Handler(void)
{
    if (signal(SIGQUIT, SIGQUIT_Handler) == SIG_ERR) {
        printf("SIGQUIT install error\n");
        exit(2);
    }
}
#endif

int main(int argc, char *argv[])
{
#ifndef _MSC_VER  //not available on windows.
    Register_SIG_QUIT_Handler();
#endif

    cmnLogger::SetMask(CMN_LOG_ALLOW_ALL);
    cmnLogger::AddChannel(std::cout, CMN_LOG_ALLOW_ALL);
    cmnLogger::SetMaskClassMatching("audio", CMN_LOG_ALLOW_ALL);
    cmnLogger::SetMaskClassMatching("AL", CMN_LOG_ALLOW_ALL);

    // add the tasks to the task manager
    mtsTaskManager * taskManager = mtsTaskManager::GetInstance();

    QApplication application(argc, argv);

    //create openAL related objects

    mtsOpenALPlay       *player     = new mtsOpenALPlay("PlayUtility", 1*cmn_ms);
    mtsOpenALRecord        *recorder   = new mtsOpenALRecord("RecUtility",   1*cmn_ms);
    mtsOpenALPlayQtComponent     *playerQT   =  new mtsOpenALPlayQtComponent("QTPlayer", 30*cmn_ms);
    mtsOpenALRecordQtComponent   *recorderQT =  new mtsOpenALRecordQtComponent("QTRecorder", 30*cmn_ms);

    sawNoteRecorderQtComponent   *noteRecorder = new sawNoteRecorderQtComponent("NoteRecorder", 30 * cmn_ms);


    taskManager->AddComponent(recorder);
    taskManager->AddComponent(player);
    taskManager->AddComponent(playerQT);
    taskManager->AddComponent(recorderQT);
    taskManager->AddComponent(noteRecorder);

    taskManager->Connect(recorderQT->GetName(), "RequiresAudioRecorder",  recorder->GetName(), "ProvidesAudioRecorder");
    taskManager->Connect(playerQT->GetName(), "RequiresAudioPlayer",    player->GetName(),  "ProvidesAudioPlayer");

    // create and start all tasks
    taskManager->CreateAll();
    taskManager->WaitForStateAll(mtsComponentState::READY);
    taskManager->StartAll();

    taskManager->WaitForStateAll(mtsComponentState::ACTIVE);

    application.setStyle("Plastique");
    recorderQT->Configure();
    playerQT->Configure();
    noteRecorder->Configure();

    application.exec();

    osaSleep(1);
    // kill all tasks and perform cleanup
    taskManager->KillAll();
    taskManager->Cleanup();

    return 0;
}
