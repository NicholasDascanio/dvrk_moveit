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
#include <cisstMultiTask/mtsManagerComponentBase.h>
#include <sawMicroScribeDigitizer/mtsMicroScribeDigitizer.h>

#include "DigitizerUserComponentFLTK.h"

// Helper macro for connect
#define CONNECT(_clientTask, _reqInt, _serverTask, _prvInt)\
    if (!localManager->Connect(_clientTask, _reqInt, _serverTask, _prvInt)) {\
        CMN_LOG_INIT_ERROR << "Failed to connect: "\
                           << ":" << _reqInt << ":" << _clientTask << " - "\
                           << ":" << _prvInt << ":" << _serverTask << std::endl;\
        exit(1);\
    }

int main()
{
    // log configuration
    cmnLogger::SetMask(CMN_LOG_ALLOW_ALL);
    cmnLogger::SetMaskFunction(CMN_LOG_ALLOW_ALL);
    cmnLogger::AddChannel(std::cout, CMN_LOG_ALLOW_NONE);
    // set the log level of detail on select components
    cmnLogger::SetMaskClass("mtsMicroScribeDigitizer", CMN_LOG_ALLOW_ALL);
	cmnLogger::SetMaskClass("DigitizerUserComponent", CMN_LOG_ALLOW_ALL);
	cmnLogger::SetMaskClass("DigitizerUserComponentFLTK", CMN_LOG_ALLOW_ALL);
    // Enable system-wide thread-safe logging
    mtsManagerLocal::SetLogForwarding(true);
	
	mtsManagerLocal * localManager = mtsManagerLocal::GetInstance();

    const double updatePeriod = 1 * cmn_ms;

	mtsMicroScribeDigitizer * digitizer;
	try {
		digitizer = new mtsMicroScribeDigitizer("Digitizer", updatePeriod);
		localManager->AddComponent(digitizer);
		digitizer->Configure();
	} catch (...) {
		std::cerr << "Failed to initialize digitizer." << std::endl;
		exit(1);
	}

	DigitizerUserComponentFLTK userUI("UserFLTK", updatePeriod);
	userUI.Configure();
	localManager->AddComponent(&userUI);

	// For system-wide logging
    CONNECT(// DigitizerUserComponentFLTK:LoggerProvided
            userUI.GetName(),
            mtsManagerComponentBase::InterfaceNames::InterfaceSystemLoggerRequired,
            // MCS:LoggerRequired
            mtsManagerComponentBase::ComponentNames::ManagerComponentServer,
            mtsManagerComponentBase::InterfaceNames::InterfaceSystemLoggerProvided);

	// Connect "Digitizer" interfaces
	CONNECT(userUI.GetName(), mtsMicroScribeDigitizer::DigitizerInterfaceName,
            digitizer->GetName(), mtsMicroScribeDigitizer::DigitizerInterfaceName);

    // create the tasks, i.e. find the commands
    localManager->CreateAll();
    localManager->StartAll();
    localManager->WaitForStateAll(mtsComponentState::ACTIVE);

    while (userUI.UIOpened()) {
        Fl::lock();
        {
            Fl::check();
        }
        Fl::unlock();
        Fl::awake();

        osaSleep(updatePeriod / 2);
    }

    CMN_LOG_RUN_VERBOSE << "UI was closed by user." << std::endl;

    // cleanup
    localManager->KillAll();
    localManager->WaitForStateAll(mtsComponentState::FINISHED, 5.0 * cmn_s);

    //localManager->Cleanup();
	delete digitizer;

    return 0;}
