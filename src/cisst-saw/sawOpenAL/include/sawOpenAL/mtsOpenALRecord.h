/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*

  Author(s):  Marcin Balicki
  Created on: 2011

  (C) Copyright 2011-2013 Johns Hopkins University (JHU), All Rights Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---

*/

#ifndef _mtsOpenALRecord_h
#define _mtsOpenALRecord_h

#include <al.h>
#include <alc.h>

#include <cisstMultiTask/mtsTaskPeriodic.h>
#include <cisstMultiTask/mtsVector.h>

// forward declaration of header classes
struct osaOpenALCAIHeader;
struct osaOpenALWAVHeader;

// Always include last!
#include <sawOpenAL/sawOpenALExport.h>

//! Records in PCM format in cisst CAI format or in WAV format.
//! The wav format has a limitation of 3GB file size when played with other players, it should be fine with cisst player
//! because we don't use the internal header size information.
//! \note the wav file has an absolute timestamp that is embedded in the first 8 bytes of the datasample.

//mono 16 bit at 441000 Hz is about 320 Mbytes per hour.

class CISST_EXPORT mtsOpenALRecord: public mtsTaskPeriodic {
    // used to control the log level
    CMN_DECLARE_SERVICES(CMN_NO_DYNAMIC_CREATION, CMN_LOG_ALLOW_ERRORS_AND_WARNINGS);

protected:
    enum {BUFFERSIZE = 50000};

    enum FileType {CAI, WAV};
    mtsBool         IsRecording;
    mtsStdString    FileName;
    FILE *          SoundFile;
    FileType        FType;

    osaOpenALCAIHeader * CAIHeader;
    osaOpenALWAVHeader * WAVHeader;
    ALCdevice * CaptureDevice;
    ALchar          SoundBuffer[BUFFERSIZE];

    int             SoundFrequency;
    int             SoundFmt;
    unsigned short  NumChannels;
    unsigned short  NumBits;
    unsigned int    BytesPerSample;

    mtsDouble       StartTime;
    mtsDouble       EndTime;
    mtsDouble       Time;
    mtsDouble       FileSize;

    const osaTimeServer * TimeServer;

    //CAI Specific
    //bool started;

    void Record(void);
    void Stop(void);
    //! /todo this needs checks to see if the device is being used?
    void SetCaptureDeviceName(const mtsStdString & deviceName);
    void SetCaptureDeviceID(const mtsUInt & deviceID);
    void GetCaptureDeviceNames(mtsStdStringVec & names) const;

    void CalcStreamVolume(ALchar * stream, int numberOfSamples);

    void PrintCAIHeader(osaOpenALCAIHeader * header);

    std::string GetALErrorString(ALenum err);
    bool    CheckALError(std::string & error);

    mtsDouble        StreamVolume;

    void OpenHeaderFile(const std::string & filename);
    void CloseHeaderFile(void);
    void WriteToHeaderFile(const double & timestamp, const int bytes);

    std::ofstream    HeaderStream;
    double           LastHeaderUpdateTime;

public:
    enum SoundFormat {MONO8, MONO16, STEREO8, STEREO16};
    // provide a name for the task and define the frequency (time
    // interval between calls to the periodic Run).  Also used to
    // populate the interface(s)
    mtsOpenALRecord(const std::string & taskName, double period, int frequency = 44100, SoundFormat fmt = mtsOpenALRecord::MONO16);
    ~mtsOpenALRecord();
    // all four methods are pure virtual in mtsTask
    void Configure(const std::string & CMN_UNUSED(filename)) {};
    void SetFileName(const mtsStdString &fileName);
    void GetFileName(mtsStdString &fileName) const;

    void Startup(void);    // set some initial values
    void Run(void);        // performed periodically
    void Cleanup(void) {}; // user defined cleanup


};

CMN_DECLARE_SERVICES_INSTANTIATION(mtsOpenALRecord);

#endif
