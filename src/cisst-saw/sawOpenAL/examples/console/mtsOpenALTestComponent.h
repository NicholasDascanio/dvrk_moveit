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

#ifndef _mtsOpenALTestComponent_h
#define _mtsOpenALTestComponent_h

#include <cisstMultiTask/mtsComponent.h>
#include <cisstMultiTask/mtsVector.h> // for mtsStdStringVec

class mtsOpenALTestComponent: public mtsComponent
{
    CMN_DECLARE_SERVICES(CMN_NO_DYNAMIC_CREATION, CMN_LOG_LOD_RUN_VERBOSE);

public:
    mtsOpenALTestComponent();
    mtsOpenALTestComponent(const std::string & taskname);
    ~mtsOpenALTestComponent();

protected:
    virtual void CreateInterfaces();

public:

    void RecordStart(const std::string & filename);
    void RecordStop();
    void PlayPause();
    void PlayStart(const std::string & filename);
    std::string GetFileName(void) const;

    void SetCaptureDeviceName(const std::string & name);
    void SetCaptureDeviceID(unsigned int id);
    void GetCaptureDeviceNames(mtsStdStringVec & names) const;

private:

    struct {
        mtsFunctionRead     GetIsRecording;
        mtsFunctionWrite    SetFileName;
        mtsFunctionRead     GetFileName;
        mtsFunctionVoid     Start;
        mtsFunctionVoid     Stop;
        mtsFunctionWrite    SetCaptureDeviceName;
        mtsFunctionWrite    SetCaptureDeviceID;
        mtsFunctionRead     GetCaptureDeviceNames;
        mtsFunctionRead     GetStreamVolume;
        mtsFunctionRead     GetTime;                //elapsed time.
    } Recorder;


    struct {
        mtsFunctionRead     GetIsPlaying;
        mtsFunctionVoid     Play;
        mtsFunctionVoid     Pause;
        mtsFunctionWrite    Seek;
        mtsFunctionRead     GetStartTime;
        mtsFunctionWrite    OpenFile;
        mtsFunctionRead     GetVolume;
        mtsFunctionWrite    SetVolume;
        mtsFunctionRead     GetTime;
        mtsFunctionRead     GetLengthInSec;

    } Player;

};

CMN_DECLARE_SERVICES_INSTANTIATION(mtsOpenALTestComponent);

#endif // _mtsOpenALTestComponent_h
