/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*

  Author(s):  Peter Kazanzides, Anton Deguet
  Created on: 2011-07-14

  (C) Copyright 2011 Johns Hopkins University (JHU), All Rights Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---
*/

#include <cisstOSAbstraction/osaSleep.h>
#include <cisstMultiTask/mtsTaskManager.h>
#include <cisstMultiTask.h>
#include <cisstCommon/cmnGetChar.h>
#include <sawMedtronicStealthlink/mtsMedtronicStealthlink.h>
#include "mtsMedtronicStealthlinkExampleComponent.h"

int main(int argc, char * argv[])
{
    // log configuration
    cmnLogger::SetMask(CMN_LOG_ALLOW_ALL);
    cmnLogger::SetMaskFunction(CMN_LOG_ALLOW_ALL);
    cmnLogger::SetMaskDefaultLog(CMN_LOG_ALLOW_ALL);
    cmnLogger::AddChannel(std::cout, CMN_LOG_ALLOW_ERRORS_AND_WARNINGS);

    // set the log level of detail on select components
    cmnLogger::SetMaskClass("cmnXMLPath", CMN_LOG_ALLOW_ALL);
    cmnLogger::SetMaskClassMatching("mts", CMN_LOG_ALLOW_ALL);

#ifdef MTS_LOGGING
    mtsManagerLocal::SetLogForwarding(true);
#endif

    std::string globalComponentManagerIP;
    int stateCollectionFlag = 0;

    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " [GlobalManagerIP] [flagForDataCollection]" << std::endl;
        std::cerr << "       GlobalManagerIP is set as 127.0.0.1 by default" << std::endl;
    }

    // Set global component manager's ip and argument type
    // If flag is not specified, or not 1, then use generic type (mtsDouble)
    if (argc == 1) {
        globalComponentManagerIP = "localhost";
    } else if (argc == 2) {
        globalComponentManagerIP = argv[1];
    }else if (argc == 3) {
        globalComponentManagerIP = argv[1];
        stateCollectionFlag = atoi(argv[2]);
    } else {
        exit(-1);
    }

    std::cout << "Starting server, IP = " << globalComponentManagerIP << " 'q' to stop " << std::endl;
    if(stateCollectionFlag)
        std::cout << "        with data collection. 's' to start/stop data collection" << std::endl;

    // Get the TaskManager instance and set operation mode
    mtsManagerLocal * componentManager;
    try {
        componentManager = mtsManagerLocal::GetInstance(globalComponentManagerIP, "ProcessServer");
    } catch (...) {
        CMN_LOG_INIT_ERROR << "Failed to initialize local component manager" << std::endl;
        return 1;
    }

    // create the components
    mtsMedtronicStealthlink * componentStealthlink = new mtsMedtronicStealthlink("Stealthlink", 50.0 * cmn_ms);
    componentStealthlink->Configure("config.xml");

    // add the components to the component manager
    componentManager->AddComponent(componentStealthlink);

    // collect all state data in csv file
    mtsCollectorState * collector;
    if(stateCollectionFlag)
    {
        collector =
                new mtsCollectorState(componentStealthlink->GetName(),
                                      componentStealthlink->GetDefaultStateTableName(),
                                      mtsCollectorBase::COLLECTOR_FILE_FORMAT_CSV);
        collector->SetName("StealthlinkStateCollector");
        //collector->AddSignal();
        collector->AddSignal("ToolData");

        componentManager->AddComponent(collector);
        collector->Connect();
    }

    // create the tasks, i.e. find the commands
    componentManager->CreateAll();
    componentManager->WaitForStateAll(mtsComponentState::READY);

    // start the periodic Run
    componentManager->StartAll();
    componentManager->WaitForStateAll(mtsComponentState::ACTIVE);

    int ch;
    bool GCMActive = true;
    bool started = false;
    while (GCMActive && ch != 'q') {
        osaSleep(5.0 * cmn_ms);
        GCMActive = componentManager->IsGCMActive();
        osaSleep(1.0 * cmn_s);

        ch = cmnGetChar();

        switch (ch) {
        case 's':
            if(started)
            {
                collector->StopCollection(0.0);
                std::cout << "Stop data collection" << std::endl;
                started = false;

            }
            else
            {
                collector->StartCollection(0.0);
                std::cout << "Start data collection" << std::endl;
                started = true;

            }
            break;

        default:
            break;
        }

    }

    if (!GCMActive) {
        CMN_LOG_RUN_ERROR << "Global Component Manager is disconnected" << std::endl;
    }

    // cleanup
    componentManager->KillAll();
    componentManager->WaitForStateAll(mtsComponentState::FINISHED, 20.0 * cmn_s);

    // delete components
    delete componentStealthlink;
    delete collector;

    componentManager->Cleanup();
    // the manager singleton needs to be cleaned up, adeguet1
    //std::cerr << "temporary hack " << CMN_LOG_DETAILS << std::endl;
    //componentManager->RemoveComponent("LCM_MCC");
    //componentManager->RemoveComponent("MCS");

    return 0;
}
