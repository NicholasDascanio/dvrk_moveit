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

#include "mtsOpenALTestComponent.h"
#include <cisstMultiTask/mtsInterfaceRequired.h>


CMN_IMPLEMENT_SERVICES(mtsOpenALTestComponent)


mtsOpenALTestComponent::mtsOpenALTestComponent():
mtsComponent("AudioTester")
{
    CreateInterfaces();
}


mtsOpenALTestComponent::mtsOpenALTestComponent(const std::string & name):
    mtsComponent(name)
{
    CreateInterfaces();
}


mtsOpenALTestComponent::~mtsOpenALTestComponent()
{
}


void mtsOpenALTestComponent::CreateInterfaces(void)
{
    mtsInterfaceRequired* required = AddInterfaceRequired("RequiresAudioRecorder", MTS_OPTIONAL);
    if (required) {
        required->AddFunction("GetIsRecording", Recorder.GetIsRecording);
        required->AddFunction("Start",          Recorder.Start);
        required->AddFunction("Stop",           Recorder.Stop);
        required->AddFunction("SetFileName",    Recorder.SetFileName);
        required->AddFunction("GetFileName",    Recorder.GetFileName);

        required->AddFunction("SetCaptureDeviceName",   Recorder.SetCaptureDeviceName);
        required->AddFunction("SetCaptureDeviceID",     Recorder.SetCaptureDeviceID);
        required->AddFunction("GetCaptureDeviceNames",  Recorder.GetCaptureDeviceNames);
        required->AddFunction("GetStreamVolume",        Recorder.GetStreamVolume);
        required->AddFunction("GetTime",                Recorder.GetTime);
    }

    required = AddInterfaceRequired("RequiresAudioPlayer", MTS_OPTIONAL);
    if (required) {
        required->AddFunction("Play",           Player.Play);
        required->AddFunction("Pause",          Player.Pause);
        required->AddFunction("Seek",           Player.Seek);
        required->AddFunction("GetStartTime",   Player.GetStartTime);
        required->AddFunction("OpenFile",       Player.OpenFile);
        required->AddFunction("GetIsPlaying",   Player.GetIsPlaying);
        required->AddFunction("GetVolume",      Player.GetVolume);
        required->AddFunction("SetVolume",      Player.SetVolume);
        required->AddFunction("GetTime",        Player.GetTime);
        required->AddFunction("GetLengthInSec", Player.GetLengthInSec);
    }
}


void mtsOpenALTestComponent::RecordStart(const std::string & filename)
{
    CMN_LOG_CLASS_RUN_VERBOSE << "RecordStart called: " << filename << std::endl;
    Recorder.SetFileName(mtsStdString(filename));
    Recorder.Start();
}


void mtsOpenALTestComponent::RecordStop()
{
    CMN_LOG_CLASS_RUN_VERBOSE << "RecordStop called" << std::endl;
    Recorder.Stop();
}


void mtsOpenALTestComponent::PlayStart(const std::string & filename)
{
    CMN_LOG_CLASS_RUN_VERBOSE << "PlayStart called: " << filename << std::endl;
    Player.OpenFile(mtsStdString(filename));
    Player.Play();
}


void mtsOpenALTestComponent::PlayPause()
{
    CMN_LOG_CLASS_RUN_VERBOSE << "PlayPause called" << std::endl;
    Player.Pause();
}


void mtsOpenALTestComponent::SetCaptureDeviceName(const std::string & name)
{
    CMN_LOG_CLASS_RUN_VERBOSE << "SetCaptureDeviceName called" << name << std::endl;
    Recorder.SetCaptureDeviceName(mtsStdString(name));
}


void mtsOpenALTestComponent::SetCaptureDeviceID(unsigned int id)
{
    CMN_LOG_CLASS_RUN_VERBOSE << "SetCaptureDeviceID called" << id << std::endl;
    Recorder.SetCaptureDeviceID(mtsUInt(id));
}


void mtsOpenALTestComponent::GetCaptureDeviceNames(mtsStdStringVec & names) const
{
    Recorder.GetCaptureDeviceNames(names);
    CMN_LOG_CLASS_RUN_VERBOSE << names.size() << " capture device names are: " <<std::endl;
    for (unsigned int i = 0; i< names.size(); i ++) {
        CMN_LOG_CLASS_RUN_VERBOSE << i << ": " << names[i] << std::endl;
    }
}

std::string mtsOpenALTestComponent::GetFileName(void) const {

     mtsStdString name;
     Recorder.GetFileName(name);
     return name.Data;

}
