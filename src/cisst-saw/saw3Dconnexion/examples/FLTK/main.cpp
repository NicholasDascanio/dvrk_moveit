/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*

  Author(s):  Marcin Balicki
  Created on: 2008-04-12

  (C) Copyright 2008-2011 Johns Hopkins University (JHU), All Rights
  Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---
*/

#include "UITask.h"
#include <saw3Dconnexion/mts3Dconnexion.h>
#include <cisstOSAbstraction/osaSleep.h>

int main(void)
{
    // Log configuration
    cmnLogger::SetMask(CMN_LOG_ALLOW_ALL);
    cmnLogger::SetMaskFunction(CMN_LOG_ALLOW_ALL);
    cmnLogger::SetMaskDefaultLog(CMN_LOG_ALLOW_ALL);
    cmnLogger::AddChannel(std::cout, CMN_LOG_ALLOW_ERRORS_AND_WARNINGS);
    cmnLogger::SetMaskClassMatching("devSpaceNavigator", CMN_LOG_ALLOW_ALL);

    // create our tasks
    mtsTaskManager * componentManager = mtsTaskManager::GetInstance();
    UITask * guiTask = new UITask("SimpleGUI", 50.0 * cmn_ms);
    guiTask->Configure();

    mts3Dconnexion * SNTask = new mts3Dconnexion("SNTask", 50.0 * cmn_ms);
    SNTask->Configure("Cube3DPolling");

    // add all tasks
    componentManager->AddComponent(guiTask);
    componentManager->AddComponent(SNTask);

    // connect: name of user, resource port, name of resource, resource interface
    componentManager->Connect("SimpleGUI", "RequiresSpaceNavigator",
                              "SNTask", "ProvidesSpaceNavigator");
    componentManager->CreateAll();
    componentManager->WaitForStateAll(mtsComponentState::READY);
    componentManager->StartAll();
    componentManager->WaitForStateAll(mtsComponentState::ACTIVE);

    // Loop until both tasks are closed, GetExitFlag also handles FLTK events
    while (!(guiTask->GetExitFlag())) {
        osaSleep(10.0 * cmn_ms);
    }

    componentManager->KillAll();
    componentManager->WaitForStateAll(mtsComponentState::ACTIVE, 5.0 * cmn_s);
    return 0;
}
