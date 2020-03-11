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

#include <QApplication>

#include <cisstCommon/cmnLogger.h>
#include <cisstStereoVision/svlInitializer.h>

#include "sdpPlayerExample.h"
#include "sdpPlayerManager.h"
#include "sdpPlayerVideo.h"
#include "sdpPlayerPlot2D.h"
#include "sdpPlayerNotes.h"

#ifdef sawPlayer_has_sawOpenAL
#include "sdpPlayerAudio.h"
#endif

int main(int argc, char *argv[])
{
   // log configuration
    cmnLogger::SetMask(CMN_LOG_ALLOW_ALL);
    cmnLogger::SetMaskDefaultLog(CMN_LOG_ALLOW_ALL);
//    cmnLogger::AddChannel(std::cout, CMN_LOG_ALLOW_ERRORS_AND_WARNINGS);
    cmnLogger::AddChannel(std::cout, CMN_LOG_ALLOW_ALL);


    // create our components
    mtsComponentManager * componentManager;
    componentManager = mtsManagerLocal::GetInstance();

    QApplication application(argc, argv);
    svlInitialize();
    sdpPlayerManager * playerManager = new sdpPlayerManager("PlayerManager", 50.0 * cmn_ms);
   // sdpPlayerExample * player = new sdpPlayerExample("Player", 1.0 * cmn_ms);
    sdpPlayerVideo * videoPlayer = new sdpPlayerVideo("VideoPlayer", 15.0 * cmn_ms);
    sdpPlayerPlot2D * plotPlayer = new sdpPlayerPlot2D("PlotPlayer", 40.0 * cmn_ms);
    sdpPlayerNotes *notePlayer  = new sdpPlayerNotes("NotePlayer", 40*cmn_ms);


#ifdef sawPlayer_has_sawOpenAL
    sdpPlayerAudio * audioPlayer = new sdpPlayerAudio("AudioPlayer", 40.0 * cmn_ms);
    componentManager->AddComponent(audioPlayer);
#endif

    componentManager->AddComponent(playerManager);
    componentManager->AddComponent(notePlayer);

  //  componentManager->AddComponent(player);
    componentManager->AddComponent(videoPlayer);
    componentManager->AddComponent(plotPlayer);

    //Connect provider/required interface of Base Class for Qt Thread
 //   componentManager->Connect(player->GetName(), "GetStatus", player->GetName(), "ProvidesStatus");
    componentManager->Connect(videoPlayer->GetName(), "GetStatus", videoPlayer->GetName(), "ProvidesStatus");
    componentManager->Connect(plotPlayer->GetName(), "GetStatus",plotPlayer->GetName(), "ProvidesStatus");
    componentManager->Connect(notePlayer->GetName(), "GetStatus",notePlayer->GetName(), "ProvidesStatus");

    //Connect componets-added interfaces for Qt Thread
    componentManager->Connect(plotPlayer->GetName(), "Get2DPlotStatus",plotPlayer->GetName(), "Provides2DPlot");

 #ifdef sawPlayer_has_sawOpenAL
    componentManager->Connect(audioPlayer->GetName(), "GetStatus", audioPlayer->GetName(), "ProvidesStatus");
 #endif

    // create the components, i.e. find the commands
    componentManager->CreateAll();
    componentManager->WaitForStateAll(mtsComponentState::READY);

    // start the periodic Run
    componentManager->StartAll();
    componentManager->WaitForStateAll(mtsComponentState::ACTIVE);

 //   playerManager->AddPlayer(player);
    playerManager->AddPlayer(videoPlayer);
    playerManager->AddPlayer(plotPlayer);
    playerManager->AddPlayer(notePlayer);


#ifdef sawPlayer_has_sawOpenAL
    playerManager->AddPlayer(audioPlayer);
#endif


    playerManager->Configure();
 //   player->Configure();
    videoPlayer->Configure();
    plotPlayer->Configure();
    notePlayer->Configure();

#ifdef sawPlayer_has_sawOpenAL
    audioPlayer->Configure();
#endif


    application.setStyle("Plastique");
    application.exec();

    componentManager->KillAll();
    componentManager->WaitForStateAll(mtsComponentState::FINISHED, 2.0 * cmn_s);

    componentManager->Cleanup();

    return 0;
}
