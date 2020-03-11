/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*

  Author(s):  Min Yang Jung
  Created on: 2011-10-06

  (C) Copyright 2011 Johns Hopkins University (JHU), All Rights Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---
*/

#include <cisstCommon/cmnGetChar.h>
#include <cisstOSAbstraction/osaSleep.h>
#include <sawMicroScribeDigitizer/mtsMicroScribeDigitizer.h>

#include "DigitizerUserComponent.h"

int main()
{
    // log configuration
    cmnLogger::SetMask(CMN_LOG_ALLOW_ALL);
    cmnLogger::SetMaskFunction(CMN_LOG_ALLOW_ALL);
    cmnLogger::AddChannel(std::cout, CMN_LOG_ALLOW_ERRORS | CMN_LOG_ALLOW_WARNINGS);
    // set the log level of detail on select components
    cmnLogger::SetMaskClass("mtsMicroScribeDigitizer", CMN_LOG_ALLOW_ALL);
	cmnLogger::SetMaskClass("DigitizerUserComponent", CMN_LOG_ALLOW_ALL);
	
	mtsManagerLocal * localManager = mtsManagerLocal::GetInstance();

	mtsMicroScribeDigitizer * digitizer;
	try {
		digitizer = new mtsMicroScribeDigitizer("Digitizer", 1 * cmn_ms);
		localManager->AddComponent(digitizer);
		digitizer->Configure();
	} catch (...) {
		std::cerr << "Failed to initialize digitizer." << std::endl;
		exit(1);
	}

    DigitizerUserComponent user("User", 1 * cmn_ms);
	user.EnableConsoleOutput();
	localManager->AddComponent(&user);

    if (!localManager->Connect(user.GetName(), mtsMicroScribeDigitizer::DigitizerInterfaceName,
                               digitizer->GetName(), mtsMicroScribeDigitizer::DigitizerInterfaceName))
    {
        std::cerr << "Failed to connect \"Digitizer\" interfaces" << std::endl;
        exit(1);
    }

    // create the tasks, i.e. find the commands
    localManager->CreateAll();
    localManager->StartAll();
    localManager->WaitForStateAll(mtsComponentState::ACTIVE);

    // loop until 'q' is pressed
    int key = ' ';
    std::cout << "Press 'q' to quit" << std::endl;
    while (key != 'q') {
        key = cmnGetChar();
    }
    std::cout << "Quitting ..." << std::endl;

    // cleanup
    localManager->KillAll();
    localManager->WaitForStateAll(mtsComponentState::FINISHED, 5.0 * cmn_s);
    //localManager->Cleanup();

    return 0;}
