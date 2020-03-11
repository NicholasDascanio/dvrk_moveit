/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*

  Author(s):  Marcin Balicki
  Created on: 2011

  (C) Copyright 2011 Johns Hopkins University (JHU), All Rights
  Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---

*/

#include <cisstCommon/cmnGetChar.h>
#include <cisstOSAbstraction/osaSleep.h>
#include <cisstMultiTask/mtsTaskManager.h>

#include <sawOpenAL/mtsOpenALRecord.h>
#include <sawOpenAL/mtsOpenALPlay.h>

#include "mtsOpenALTestComponent.h"


int main(int argc, char *argv[])
{
    cmnLogger::SetMask(CMN_LOG_ALLOW_ALL);
    cmnLogger::SetMaskFunction(CMN_LOG_ALLOW_ALL);
    cmnLogger::SetMaskDefaultLog(CMN_LOG_ALLOW_ALL);
    cmnLogger::AddChannel(std::cout, CMN_LOG_ALLOW_ALL);
    cmnLogger::SetMaskClassMatching("mtsOpenAL", CMN_LOG_ALLOW_ALL);

    // add the components to the component manager
    mtsComponentManager * componentManager = mtsComponentManager::GetInstance();

    //create openAL related objects
    mtsOpenALRecord * recorder = new mtsOpenALRecord("RecUtility", 5.0 * cmn_ms);
    mtsOpenALPlay * player = new mtsOpenALPlay("PlayUtility", 5.0 * cmn_ms);
    mtsOpenALTestComponent * tester = new mtsOpenALTestComponent("Tester");

    componentManager->AddComponent(recorder);
    componentManager->AddComponent(player);
    componentManager->AddComponent(tester);

    componentManager->Connect(tester->GetName(), "RequiresAudioRecorder",
                              recorder->GetName(), "ProvidesAudioRecorder");
    componentManager->Connect(tester->GetName(), "RequiresAudioPlayer",
                              player->GetName(),  "ProvidesAudioPlayer");

    // create and start all components
    componentManager->CreateAll();
    componentManager->WaitForStateAll(mtsComponentState::READY);
    componentManager->StartAll();
    componentManager->WaitForStateAll(mtsComponentState::ACTIVE);

    std::cout << "Choose your audio capture device" << std::endl;
    mtsStdStringVec names;
    tester->GetCaptureDeviceNames(names);

//    int ch = 0;
//    ch = cmnGetChar();
//    int i = atoi((char *) (&ch));
    tester->SetCaptureDeviceID(0);

    std::string testFile("test.wav");
    tester->RecordStart(testFile);


    std::cout << "Recording" << std::endl;

    osaSleep(5 * cmn_s);

    tester->RecordStop();

    osaSleep(1.0 * cmn_s);
    tester->PlayStart( tester->GetFileName());

    std::cout << "Playing" << std::endl;


    osaSleep(6.0 * cmn_s);
    tester->PlayPause();

    // kill all components and perform cleanup
    componentManager->KillAll();
    componentManager->Cleanup();

    return 0;
}
