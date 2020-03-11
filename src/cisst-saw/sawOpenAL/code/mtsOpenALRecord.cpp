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

#include <sawOpenAL/mtsOpenALRecord.h>

#include <cisstOSAbstraction/osaGetTime.h>
#include <cisstMultiTask/mtsInterfaceProvided.h>
#include <sawOpenAL/osaOpenALAudioTypes.h>

#include <fstream>


// required to implement the class services, see cisstCommon
CMN_IMPLEMENT_SERVICES(mtsOpenALRecord);

mtsOpenALRecord::mtsOpenALRecord(const std::string & taskName, double period, int frequency, SoundFormat fmt):
    mtsTaskPeriodic(taskName, period, false, 5000)
{

    std::string err;
    CheckALError(err);

    FileSize   = 0;
    CAIHeader = 0;
    WAVHeader = 0;
    CaptureDevice = 0;
    
    SoundFile = 0;
    FileName = "";
    FType = mtsOpenALRecord::CAI;
    SoundFrequency = frequency;

    if (fmt == mtsOpenALRecord::MONO8) {
        SoundFmt = AL_FORMAT_MONO8;
        NumChannels = 1;
        NumBits = 8;
    }
    else if (fmt == mtsOpenALRecord::MONO16) {
        SoundFmt = AL_FORMAT_MONO16;
        NumChannels = 1;
        NumBits = 16;
    }
    else if (fmt == mtsOpenALRecord::STEREO8) {
        SoundFmt = AL_FORMAT_STEREO8;
        NumChannels = 2;
        NumBits = 8;
    }
    else if ( fmt == mtsOpenALRecord::STEREO16) {
        SoundFmt = AL_FORMAT_STEREO16;
        NumChannels = 2;
        NumBits = 16;
    }

    BytesPerSample = (NumChannels * NumBits/8 );

    mtsInterfaceProvided * provided = AddInterfaceProvided("ProvidesAudioRecorder");
    if (provided) {
        StateTable.AddData(IsRecording, "IsRecording");
        StateTable.AddData(FileName,    "FileName");
        StateTable.AddData(StreamVolume,   "StreamVolume");
        StateTable.AddData(Time,        "Time");
        StateTable.AddData(FileSize,    "FileSize");

        provided->AddCommandReadState(StateTable, IsRecording,      "GetIsRecording");
        provided->AddCommandReadState(StateTable, StreamVolume,      "GetStreamVolume");
        provided->AddCommandReadState(StateTable, Time,             "GetTime");
        provided->AddCommandReadState(StateTable, FileSize,         "GetFileSize");

        // provided->AddCommandWriteState(StateTable, FileName,     "SetFileName");
        provided->AddCommandWrite(&mtsOpenALRecord::SetFileName,  this, "SetFileName", mtsStdString());
        provided->AddCommandRead(&mtsOpenALRecord::GetFileName,  this, "GetFileName", mtsStdString());
        provided->AddCommandVoid(&mtsOpenALRecord::Record, this,    "Start");
        provided->AddCommandVoid(&mtsOpenALRecord::Stop,   this,    "Stop");
        provided->AddCommandWrite(&mtsOpenALRecord::SetCaptureDeviceName, this,    "SetCaptureDeviceName", mtsStdString());
        provided->AddCommandWrite(&mtsOpenALRecord::SetCaptureDeviceID, this,      "SetCaptureDeviceID", mtsUInt());
        provided->AddCommandRead(&mtsOpenALRecord::GetCaptureDeviceNames,this,     "GetCaptureDeviceNames", mtsStdStringVec());
    }

    StreamVolume = 0;

    //    const ALCchar *szDefaultCaptureDevice = alcGetString(NULL, ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER);
    //    CMN_LOG_CLASS_RUN_VERBOSE<<"\nDefault Capture Device is"<<szDefaultCaptureDevice<<std::endl;
    //    captureDevice = alcCaptureOpenDevice(szDefaultCaptureDevice, soundFrequency, soundFmt, BUFFERSIZE);
    //    CMN_LOG_CLASS_RUN_VERBOSE<<captureDevice<<" <- capture device pointer"<<std::endl;


    TimeServer = &mtsTaskManager::GetInstance()->GetTimeServer();
}


mtsOpenALRecord::~mtsOpenALRecord()
{

    if (IsRecording) {
        Stop();
    }

    if (SoundFile) {
        fclose(SoundFile);
    }

    delete CAIHeader;
    delete WAVHeader;

    if (CaptureDevice) {
        alcCaptureStop(CaptureDevice);
        alcCaptureCloseDevice(CaptureDevice);
    }

    //if (!alutExit())
    //    CMN_LOG_CLASS_RUN_ERROR<<"ALUT Failed to shutdown!"<<std::endl;
}


void mtsOpenALRecord::Startup(void)
{
    CMN_LOG_CLASS_INIT_VERBOSE << "Startup: starting..." << std::endl;
    SetCaptureDeviceID(mtsUInt(0));
}


void mtsOpenALRecord::Run(void)
{
    ProcessQueuedCommands();

    ALint iSamplesAvailable = 0;
    alcGetIntegerv(CaptureDevice, ALC_CAPTURE_SAMPLES, sizeof(ALint), &iSamplesAvailable);

    if (iSamplesAvailable == 0) {
        return;
    }

    if (iSamplesAvailable * BytesPerSample > BUFFERSIZE) {
        CMN_LOG_CLASS_RUN_ERROR << "Run: buffer overflow with " << iSamplesAvailable << " samples" << std::endl;
        iSamplesAvailable = BUFFERSIZE;
    }

    alcCaptureSamples(CaptureDevice, SoundBuffer, iSamplesAvailable);
    CalcStreamVolume(SoundBuffer, iSamplesAvailable);

    if (IsRecording) {

        fwrite(SoundBuffer, iSamplesAvailable * BytesPerSample, 1, SoundFile);

        if (FType == mtsOpenALRecord::WAV) {
            WAVHeader->lDataSize += iSamplesAvailable * BytesPerSample;
        }

        FileSize += iSamplesAvailable * BytesPerSample;
        double currentTime = TimeServer->GetAbsoluteTimeInSeconds();
        Time  = currentTime - StartTime;

        if (LastHeaderUpdateTime + 1 < currentTime) {
            LastHeaderUpdateTime = currentTime;
            WriteToHeaderFile(currentTime, WAVHeader->lDataSize);
        }
    }
    else {
        //  alcGetIntegerv(captureDevice, ALC_CAPTURE_SAMPLES, 4, &iSamplesAvailable);
    }
}


void mtsOpenALRecord::Record(void)
{
    if (!CaptureDevice) {
        CMN_LOG_CLASS_RUN_ERROR << "Record: no device to record from" << std::endl;
        IsRecording = false;
    }
    //    std::cout<<"time "<<::osaGetTime()<<std::endl;
    if (!IsRecording) {
        int pos = FileName.Data.find_last_of('.');
        if (pos == std::string::npos
            || pos == 0
            || (SoundFile = ::fopen(FileName.Data.c_str(), "wb")) == 0) {
            CMN_LOG_CLASS_RUN_ERROR << "Record: file name is not valid! : '" << FileName.Data << "'" <<std::endl;
        }
        else {
            //alcCaptureStop(captureDevice); //reset the buffer?
            //osaSleep(0.15);
            //ALint iSamplesAvailable = 0;
            //alcGetIntegerv(captureDevice, ALC_CAPTURE_SAMPLES, 4, &iSamplesAvailable);
            //            alcCaptureSamples(captureDevice, soundBuffer, iSamplesAvailable);

            //start again.
            //            alcCaptureStart(captureDevice);
            StartTime = TimeServer->GetAbsoluteTimeInSeconds();

            //            std::string err;
            //            if (CheckALError(err)) {
            //                CMN_LOG_CLASS_RUN_ERROR<<"Can't record on this device: "<< FileName.Data <<std::endl;
            //                StartTime = 0;
            //                ::fflush(soundFile);
            //                ::fclose(soundFile);
            //                soundFile = NULL;

            //                return;
            //            }

            OpenHeaderFile(FileName);
            WriteToHeaderFile(StartTime, 0);
            LastHeaderUpdateTime = StartTime;
            IsRecording = true;

            CMN_LOG_CLASS_RUN_VERBOSE << "Run: recording to: " << FileName.Data << std::endl;

            // CAI
            if (FileName.Data.substr(pos+1) == "cai") {
                FType = mtsOpenALRecord::CAI;
                delete CAIHeader;
                CAIHeader = new osaOpenALCAIHeader;
                //started=false;
                CAIHeader->nChannels         = NumChannels;
                CAIHeader->nBytesPerSample   = NumBits/8;
                CAIHeader->frequency         = SoundFrequency;
                CAIHeader->StartTime         = StartTime;
                CAIHeader->EndTime           = 0;

                fwrite(CAIHeader, sizeof(osaOpenALCAIHeader), 1, SoundFile);

                PrintCAIHeader(CAIHeader);
                FileSize = sizeof(osaOpenALCAIHeader);
            }

            // WAV
            else if (FileName.Data.substr(pos + 1) == "wav") {
                FType = mtsOpenALRecord::WAV;
                delete WAVHeader;
                WAVHeader = new osaOpenALWAVHeader;

                //Set up wav header
                sprintf(WAVHeader->szRIFF, "RIFF");
                WAVHeader->lRIFFSize = 0;
                sprintf(WAVHeader->szWave, "WAVE");
                sprintf(WAVHeader->szFmt, "fmt ");
                WAVHeader->lFmtSize = sizeof(WAVEFORMATEX); //18
                WAVHeader->wfex.wFormatTag = 1;
                WAVHeader->wfex.nChannels = NumChannels;
                WAVHeader->wfex.nSamplesPerSec = SoundFrequency;
                WAVHeader->wfex.wBitsPerSample = NumBits;
                WAVHeader->wfex.nBlockAlign = WAVHeader->wfex.nChannels * WAVHeader->wfex.wBitsPerSample / 8;
                WAVHeader->wfex.nAvgBytesPerSec = WAVHeader->wfex.nSamplesPerSec * WAVHeader->wfex.nBlockAlign;
                WAVHeader->wfex.cbSize = 0;

//                sprintf(WAVHeader->szTime, "abTM");
//                WAVHeader->lTimeSize = 8;
//                WAVHeader->timeStamp = StartTime.Data;

                sprintf(WAVHeader->szData, "data");
                WAVHeader->lDataSize = 0;
                fwrite(WAVHeader, sizeof(osaOpenALWAVHeader), 1, SoundFile);
                FileSize = sizeof(osaOpenALWAVHeader);

            }
            else {
                CMN_LOG_CLASS_RUN_VERBOSE << "Run: audio type not implemented" << std::endl;
            }
        }
    }
    else {
	    CMN_LOG_CLASS_RUN_WARNING << "Run: already recording. First call Stop()" << std::endl;
	}
}


void mtsOpenALRecord::Stop(void)
{
    if (IsRecording) {

        EndTime = TimeServer->GetAbsoluteTimeInSeconds();

        //alcCaptureStop(captureDevice);
        IsRecording = false;
        if (FType == mtsOpenALRecord::CAI) {
            CMN_LOG_CLASS_RUN_VERBOSE << "Stop: stopping CAI" << std::endl;
            /*double timeStamp=::osaGetTime();
              fwrite(&timeStamp, sizeof(double), 1, soundFile);*/
            delete CAIHeader;
            CAIHeader = 0;
        }
        else if (FType == mtsOpenALRecord::WAV) {
            CMN_LOG_CLASS_RUN_VERBOSE << "Stop: stopping WAV" << std::endl;
            ::fseek(SoundFile, 4, SEEK_SET);
            u_int32_t iSize = WAVHeader->lDataSize + sizeof(osaOpenALWAVHeader);
            ::fwrite(&iSize, 4, 1, SoundFile);
            //write the data size at the end of the header.
            ::fseek(SoundFile, sizeof(osaOpenALWAVHeader) - 4, SEEK_SET);
            ::fwrite(&WAVHeader->lDataSize, 4, 1, SoundFile);
            ::fflush(SoundFile);
            delete WAVHeader;
            WAVHeader = 0;
        }

        ::fflush(SoundFile);
        ::fclose(SoundFile);
        SoundFile = 0;

        CMN_LOG_CLASS_RUN_WARNING << std::setprecision(3) << std::fixed
                                  << "Stop: StarTime: "<< StartTime.Data
                                  << " EndTime: "<< EndTime.Data
                                  << " Length: " << EndTime.Data-StartTime.Data << " seconds" << std::endl;
        CloseHeaderFile();
    }
    else {
        CMN_LOG_CLASS_RUN_VERBOSE << "Stop: not recording, nothing to stop." << std::endl;
    }
}


void mtsOpenALRecord::SetFileName(const mtsStdString & fileName)
{
    CMN_LOG_CLASS_RUN_VERBOSE << "SetFileName: " << fileName.Data << std::endl;
    std::string date;
    osaGetDateTimeString(date);

    size_t found = fileName.Data.find_last_of(".");

    if (found) {
        FileName = fileName.Data.substr(0,found) + "_" + date + fileName.Data.substr(found) ;
        CMN_LOG_CLASS_RUN_VERBOSE << "SetFileName with date: " << FileName.Data << std::endl;
    }

    Time = 0;
}

void mtsOpenALRecord::GetFileName(mtsStdString & fileName) const
{

    CMN_LOG_CLASS_RUN_VERBOSE << "GetFileName: " << fileName.Data << std::endl;

    fileName = FileName;

}



void mtsOpenALRecord::GetCaptureDeviceNames(mtsStdStringVec & names) const
{
    // Get list of available Capture Devices
    const ALchar *pDeviceList = alcGetString(NULL, ALC_CAPTURE_DEVICE_SPECIFIER);
    if (pDeviceList)
        {
            CMN_LOG_CLASS_RUN_VERBOSE << "GetCaptureDeviceNames: available capture devices are: " <<std::endl;
            int i = 0;
            while (*pDeviceList)
                {
                    CMN_LOG_CLASS_RUN_VERBOSE << " - " << i++ << ": " << pDeviceList << std::endl;
                    names.resize(i);
                    names[i-1]= std::string(pDeviceList);
                    pDeviceList += strlen(pDeviceList) + 1;
                }
        }

    // Get the name of the 'default' capture device
    const ALCchar * szDefaultCaptureDevice = alcGetString(NULL, ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER);
    CMN_LOG_CLASS_RUN_VERBOSE <<"GetCaptureDeviceNames: default capture device is " << szDefaultCaptureDevice << std::endl;
}


void mtsOpenALRecord::SetCaptureDeviceName(const mtsStdString & deviceName)
{
    CMN_LOG_CLASS_RUN_VERBOSE << "SetCaptureDeviceName: " << deviceName.Data << std::endl;
    if (CaptureDevice) {
        alcCaptureCloseDevice(CaptureDevice);
    }
    CaptureDevice = alcCaptureOpenDevice(deviceName.Data.c_str(), SoundFrequency, SoundFmt, BUFFERSIZE);
    if (!CaptureDevice) {
        CMN_LOG_CLASS_RUN_ERROR << "SetCaptureDeviceName: could not open capture device: " << deviceName.Data << std::endl;
    }
    else {
        alcCaptureStart(CaptureDevice);
        std::string err;
        if (CheckALError(err)) {
            CMN_LOG_CLASS_RUN_ERROR << "SetCaptureDeviceName: can't record on this device: " << deviceName.Data << std::endl;
            return;
        }
    }
}


void mtsOpenALRecord::SetCaptureDeviceID(const mtsUInt & deviceID)
{
    CMN_LOG_CLASS_RUN_VERBOSE << "SetCaptureDeviceID: " << deviceID.Data << std::endl;

    // Get list of available Capture Devices
    const ALchar *pDeviceList = alcGetString(NULL, ALC_CAPTURE_DEVICE_SPECIFIER);
    if (pDeviceList) {
	    int i = 0;
	    while (*pDeviceList)
            {
                if (i == deviceID) {
                    SetCaptureDeviceName(mtsStdString(std::string(pDeviceList)));
                }
                i++;
                pDeviceList += strlen(pDeviceList) + 1;
            }
	}
}


void mtsOpenALRecord::PrintCAIHeader(osaOpenALCAIHeader * header) {
    CMN_LOG_CLASS_RUN_VERBOSE << "Header size:    " << sizeof(osaOpenALCAIHeader) << std::endl;
    CMN_LOG_CLASS_RUN_VERBOSE << "Frequency:      " << header->frequency << std::endl;
    int d = header->nBytesPerSample;
    CMN_LOG_CLASS_RUN_VERBOSE << "BytesPerSample: " << d << std::endl;
    d = header->nChannels;
    CMN_LOG_CLASS_RUN_VERBOSE << "Channels:       " << d << std::endl;
    CMN_LOG_CLASS_RUN_VERBOSE << "StartTime:      " << header->StartTime << std::endl;
    CMN_LOG_CLASS_RUN_VERBOSE << "EndTime:        " << header->EndTime << std::endl;
}


std::string mtsOpenALRecord::GetALErrorString(ALenum err)
{
    switch(err)
        {
        case ALC_NO_ERROR:
            return std::string("AL_NO_ERROR");
            break;

        case ALC_INVALID_DEVICE:
            return std::string("ALC_INVALID_DEVICE");
            break;

        case ALC_INVALID_CONTEXT:
            return std::string("ALC_INVALID_CONTEXT");
            break;

        case ALC_INVALID_ENUM:
            return std::string("ALC_INVALID_ENUM");
            break;

        case ALC_INVALID_VALUE:
            return std::string("ALC_INVALID_VALUE");
            break;

        case ALC_OUT_OF_MEMORY:
            return std::string("ALC_OUT_OF_MEMORY");
            break;
        };
}


bool mtsOpenALRecord::CheckALError(std::string &error)
{
    ALenum result;

    if ((result = alGetError()) != AL_NO_ERROR) {
        error = GetALErrorString(result);
        return true;
    }
    else {
        return false;
    }
}


void mtsOpenALRecord::CalcStreamVolume(ALchar * stream, int numberOfSamples)
{
    StreamVolume = 0;
    if (NumBits == 16) {
        short *buf = (short *) stream;
        int v = 0;
        for (unsigned int i = 0; i < numberOfSamples ; i++) {
            v = abs(buf[i]);
            if (StreamVolume < v)
                StreamVolume = v;
        }
        //normalize 0-100;
        StreamVolume = (StreamVolume / SHRT_MAX);
    }
    else {
        //8bit is 0-255 with 128 being zero
        int v = 0;
        for (unsigned int i = 0; i < numberOfSamples ; i++) {
            v = stream[i];
            v = abs(v-127);
            if (StreamVolume < v)
                StreamVolume = v;
        }
        //normalize 0-100;
        StreamVolume = (StreamVolume / CHAR_MAX);
    }
}


void mtsOpenALRecord::OpenHeaderFile(const std::string & filename)
{
    HeaderStream.close();
    HeaderStream.clear();

    std::string headerFName = filename + ".txt";
    HeaderStream.open(headerFName.c_str(), std::ios::out | std::ios::app);

    if (HeaderStream.fail()) {
        CMN_LOG_CLASS_RUN_ERROR << "OpenHeaderFile: can't open file " << HeaderStream << std::endl;
    }
    else {
        HeaderStream.precision(4);
        HeaderStream.setf(std::ios::fixed);
        HeaderStream << "Timestamp BytesSinceStart" << std::endl;
    }
}


void mtsOpenALRecord::CloseHeaderFile(void)
{
    HeaderStream.close();
    HeaderStream.clear();
}


void mtsOpenALRecord::WriteToHeaderFile(const double & timestamp, const int bytes)
{
    HeaderStream << timestamp << " " << bytes << std::endl;
}
