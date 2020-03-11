/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */
/*

  Author(s):  Anton Deguet
  Created on: 2009-03-27

  (C) Copyright 2009-2011 Johns Hopkins University (JHU), All Rights
  Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---
*/

#include <cisstOSAbstraction/osaSleep.h>
#include <sawSartoriusScale/mtsSartoriusSerial.h>

#include "displayTask.h"
#include "displayUI.h"

int main(int argc, char ** argv)
{
    // log configuration
    cmnLogger::SetMask(CMN_LOG_ALLOW_ALL);
    cmnLogger::SetMaskFunction(CMN_LOG_ALLOW_ALL);
    cmnLogger::SetMaskDefaultLog(CMN_LOG_ALLOW_ALL);
    cmnLogger::AddChannel(std::cout, CMN_LOG_ALLOW_ERRORS_AND_WARNINGS);

    // specify a higher, more verbose log level for these classes
    cmnLogger::SetMaskClass("mtsSartoriusSerial", CMN_LOG_ALLOW_ALL);
    cmnLogger::SetMaskClass("osaSerialPort", CMN_LOG_ALLOW_ALL);

    std::string portName;
    int portNumber;
    bool useFullPortName;
    if (argc == 1) {
#if (CISST_OS == CISST_WINDOWS)
        useFullPortName = false;
        portNumber = 4;
#elif (CISST_OS == CISST_DARWIN)
        useFullPortName = true;
        portName = "/dev/tty.KeySerial1";
#else
        useFullPortName = false;
        portName = 1;
#endif
    } else {
        if (argc == 2) {
            useFullPortName = true;
            portName = argv[1];
        } else {
            std::cerr << "Error, needs to provide at most one parameter: port name" << std::endl;
            return -1;
        }
    }

    // create our two tasks
    const long PeriodDisplay = 10; // in milliseconds
    mtsComponentManager * componentManager = mtsComponentManager::GetInstance();
    displayTask * displayTaskObject =
        new displayTask("Display", PeriodDisplay * cmn_ms);
    displayTaskObject->Configure();
    componentManager->AddComponent(displayTaskObject);

    mtsSartoriusSerial * scaleObject;
    if (useFullPortName) {
        std::cout << "Creating Sartorius Scale using serial port " << portName << std::endl;
        scaleObject = new mtsSartoriusSerial("Sartorius", portName);
    } else {
        std::cout << "Creating Sartorius Scale using serial port " << portNumber << std::endl;
        scaleObject = new mtsSartoriusSerial("Sartorius", portNumber);
    }

	componentManager->AddComponent(scaleObject);

    // connect the tasks
    componentManager->Connect("Display", "Scale", "Sartorius", "Scale");

    componentManager->CreateAll();
    componentManager->WaitForStateAll(mtsComponentState::READY, 2.0 * cmn_s);
    componentManager->StartAll();
    componentManager->WaitForStateAll(mtsComponentState::ACTIVE, 2.0 * cmn_s);

    // wait until the close button of the UI is pressed
    while (!displayTaskObject->GetExitFlag()) {
        osaSleep(20.0 * cmn_ms);
    }

    // cleanup
    componentManager->KillAll();
    componentManager->WaitForStateAll(mtsComponentState::FINISHED, 2.0 * cmn_s);
    componentManager->Cleanup();

    return 0;
}
