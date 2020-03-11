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

#include <sawOpenAL/osaOpenAL.h>
#include <sawOpenAL/osaOpenALAudioTypes.h>

#include <cisstMultiTask/mtsInterfaceProvided.h>

#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <fstream>

// required to implement the class services, see cisstCommon
CMN_IMPLEMENT_SERVICES(osaOpenAL);

osaOpenAL::osaOpenAL():
    cmnGenericObject()
{
    SoundSettings = new osaOpenALCAIHeader;
    CAIHeader = new osaOpenALCAIHeader;
    WAVHeader = new osaOpenALWAVHeader;

    SoundFile = 0;
    FileName =  "";
    FType = osaOpenAL::CAI;
    StartTimeAbsolute = 0;
    Volume = 50;
    Time = 0;

    IsPlaying = false;

    Data = 0;

    TimeServer = &mtsTaskManager::GetInstance()->GetTimeServer();
}


osaOpenAL::~osaOpenAL()
{
    delete SoundSettings;
    SoundSettings = 0;
    delete CAIHeader;
    CAIHeader = 0;
    delete WAVHeader;
    WAVHeader = 0;

    if (SoundFile){
        fclose(SoundFile);
    }

    if (Data){
        alDeleteBuffers(1, SoundBuffer);
        alDeleteSources(1, SoundSource);
        delete[] Data;
    }

    if (!CloseOpenAL()) {
        CMN_LOG_CLASS_RUN_ERROR << "Failed to close OpenAL" << std::endl;
    }

}


void osaOpenAL::Startup(void)
{
    CMN_LOG_CLASS_RUN_VERBOSE << "osaOpenAL starting ..." << std::endl;
    if (!InitOpenAL()) {
        CMN_LOG_CLASS_INIT_ERROR << "Failed to initialize OpenAL" << std::endl;
    }

}


void osaOpenAL::Run(void)
{

    ALint iState = 0;

    if (SoundFile) {

        //Time =  TimeServer->GetAbsoluteTimeInSeconds() - StartTime;
        //alGetSourcef( soundSource[0], AL_SEC_OFFSET, &altime);  //nearest second is not good enough.
        ALint  currentPos(0);
        alGetSourcei (SoundSource[0], AL_SAMPLE_OFFSET, &currentPos);

        Time            = SoundSettings->StartTime + CalcStreamTime(currentPos);
        StreamVolume    = CalcStreamVolume(currentPos);
        StreamVolume.Timestamp() = Time;

        alGetSourcei(SoundSource[0], AL_SOURCE_STATE, &iState);
        if (iState != AL_PLAYING ){
            if (IsPlaying == true) {
                CMN_LOG_CLASS_RUN_VERBOSE << "Run: play stopped at: " << Time.Data << std::endl;
                IsPlaying = false;
            }
        } else {

        }
    } else  {
        IsPlaying = false;
    }
}


void osaOpenAL::Pause(void)
{
    CMN_LOG_CLASS_RUN_DEBUG << "Pause called" << std::endl;
    if (IsPlaying && (SoundFile)){
        alSourcePause(SoundSource[0]);
        IsPlaying = false;
    }
}


void osaOpenAL::Seek(const mtsDouble & time)
{
    CMN_LOG_CLASS_RUN_DEBUG << "Seek called for " << time.Data <<std::endl;

    if (SoundFile) {
        bool tmpWasPlaying = IsPlaying;
        double alTime = time - StartTimeAbsolute;

        if (FType == osaOpenAL::CAI
                || FType == osaOpenAL::WAV) {

            if (alTime > LengthInSec) {
                CMN_LOG_CLASS_RUN_VERBOSE << "Seek reached the end (" << StartTimeAbsolute + LengthInSec
                                          << ") for requested time: "<< time.Data << std::endl;
            } else if (alTime < 0) {
                CMN_LOG_CLASS_RUN_VERBOSE << "Seek reached the beginning (" <<StartTimeAbsolute
                                          <<") for requested time: " << time.Data << std::endl;
            } else {

                int samplePos = CalcStreamPos(alTime);

                //alSourceStop(SoundSource[0]);
                //                ALfloat t = alTime;
                //! \todo Convert the time to sample offset because this call might be too low of a resolution.
                //                alSourcef(SoundSource[0], AL_SEC_OFFSET, t);
                alSourcei(SoundSource[0], AL_SAMPLE_OFFSET, samplePos);
                //alSourcePlay(SoundSource[0]);
                //alSourcePause(SoundSource[0]);
            }
        }
    }
}


void osaOpenAL::Play(void)
{
    if (!IsPlaying && (SoundFile)) {
        alSourcef(SoundSource[0], AL_GAIN, Volume.Data);
        alSourcePlay(SoundSource[0]);
        IsPlaying = true;
        CMN_LOG_CLASS_RUN_DEBUG << "Play called" << std::endl;
    } else {
        if (!SoundFile) {
            CMN_LOG_CLASS_RUN_WARNING << "Play called but no file to play" << std::endl;
        } else {
            CMN_LOG_CLASS_RUN_DEBUG << "Play called but already playing" << std::endl;
        }
    }
}


void osaOpenAL::Stop(void)
{
    CMN_LOG_CLASS_RUN_DEBUG << "Stop called" << std::endl;

    ALint iState;
    alGetSourcei(SoundSource[0], AL_SOURCE_STATE, &iState);
    if (iState == AL_PLAYING
            || iState == AL_PAUSED) {
        IsPlaying = false;
    }
    alSourceStop(SoundSource[0]);
}


void osaOpenAL::OpenFile(const mtsStdString & fName)
{
    Stop();

    if (Data) {
        alDeleteBuffers(1, SoundBuffer);
        alDeleteSources(1, SoundSource);
        delete[] Data;
        Data = 0;
    }

    if (SoundFile) {
        CMN_LOG_CLASS_RUN_DEBUG << "Closing file" <<std::endl;
        fclose(SoundFile);
        SoundFile = 0;
    }

    double startTime = 0;
    double bytesPerWholeSample = 0;

    FileName = fName.Data;
    int pos = FileName.Data.find_last_of('.');
    if (pos == std::string::npos
            || pos == 0
            || (SoundFile = ::fopen(FileName.Data.c_str(), "rb")) == 0 ) {
        CMN_LOG_CLASS_RUN_ERROR << "OpenFile: file name is not valid: " << FileName.Data << std::endl;
        Time = 0;
        return;
    }

    // using CAI format
    if (FileName.Data.substr(pos + 1) == "cai") {
        CMN_LOG_CLASS_RUN_VERBOSE << "FileOpen: opening CAI file "<< FileName.Data << std::endl;
        FType = osaOpenAL::CAI;

        if (::fread(CAIHeader, 1, sizeof(osaOpenALCAIHeader), SoundFile)) {
            unsigned int tmpCurrent = ftell(SoundFile);

            fseek(SoundFile, 0, SEEK_END);
            NumDataBytes = ftell(SoundFile) - tmpCurrent;
            fseek(SoundFile, tmpCurrent, SEEK_SET);
            //this is the whole file in memory.

            double bytesPerWholeSample =  CAIHeader->nBytesPerSample * CAIHeader->nChannels;

            LengthInSec = (double)NumDataBytes / bytesPerWholeSample  / (double)CAIHeader->frequency;
            CMN_LOG_CLASS_RUN_VERBOSE << "OpenFile: file length is : "
                                      << LengthInSec.Data << " seconds" << std::endl;

            StartTimeAbsolute = CAIHeader->StartTime;
            CMN_LOG_CLASS_RUN_VERBOSE << std::setprecision(3) << std::fixed
                                      <<"OpenFile: absolute start time: " << StartTimeAbsolute.Data << std::endl;

            Data = new char[NumDataBytes];

            ::fread(Data, 1, NumDataBytes, SoundFile);

            ALenum tmpFormat;

            if (CAIHeader->nChannels == 1) {
                if (CAIHeader->nBytesPerSample == 1) {
                    tmpFormat = AL_FORMAT_MONO8;
                } else {
                    tmpFormat = AL_FORMAT_MONO16;
                }
            } else {
                if (CAIHeader->nBytesPerSample == 1) {
                    tmpFormat = AL_FORMAT_STEREO8;
                } else {
                    tmpFormat = AL_FORMAT_STEREO16;
                }
            }

            *SoundSettings = *CAIHeader;

            alGetError();
            alGenBuffers(1, SoundBuffer);
            std::string err;
            if (CheckALError(err)) {
                CMN_LOG_CLASS_RUN_ERROR << "OpenFile: OpenAL error: " << err
                                        <<" - while generating data BUFFER" << std::endl;
            }
            alBufferData(SoundBuffer[0], tmpFormat, Data, NumDataBytes, CAIHeader->frequency);
            alGenSources(1, SoundSource);

            if (CheckALError(err)) {
                CMN_LOG_CLASS_RUN_ERROR << "OpenFile: OpenAL error: " << err
                                        << "- while creating data buffer and SOURCES" << std::endl;
            }

            alSourcei(SoundSource[0], AL_BUFFER, SoundBuffer[0]);
        }

        Seek(0);
        RangeChangedEvent();
    }
    // WAV format
    else if (FileName.Data.substr(pos+1) == "wav") {
        CMN_LOG_CLASS_RUN_VERBOSE << "FileOpen: opening WAV file " << FileName.Data << std::endl;
        FType = osaOpenAL::WAV;

        //! \todo at the moment we can't read other wav files. To fix, look for tags first.
        //Test if we are using the old waveheader

        osaOpenALCISSTWAVHeader  cisstWAVHeader;

        ::fread(&cisstWAVHeader, 1, sizeof(osaOpenALCISSTWAVHeader), SoundFile);

        if (strcmp(cisstWAVHeader.szTime, "abTM") == 0) {

            CMN_LOG_CLASS_RUN_WARNING << "Using old wav file version" <<std::endl;
            unsigned int tmpCurrent = ftell(SoundFile);

            fseek(SoundFile, 0, SEEK_END);
            NumDataBytes = ftell(SoundFile) - tmpCurrent;
            fseek(SoundFile, tmpCurrent, SEEK_SET);
            //this is the whole file in memory.

            if (cisstWAVHeader.wfex.wFormatTag != 1) {
                CMN_LOG_CLASS_RUN_ERROR << "OpenFile: wrong WAV format for file: " << FileName.Data << std::endl;
            }

            SoundSettings->nBytesPerSample   = cisstWAVHeader.wfex.wBitsPerSample / 8;
            SoundSettings->nChannels         = cisstWAVHeader.wfex.nChannels;
            SoundSettings->frequency         = cisstWAVHeader.wfex.nSamplesPerSec;

            bytesPerWholeSample =  cisstWAVHeader.wfex.nBlockAlign;  // WAV_Header->wfex.nChannels * WAV_Header->wfex.wBitsPerSample / 8
            startTime =   cisstWAVHeader.timeStamp;
        }
        else {

            fseek(SoundFile, 0, SEEK_SET);
            ::fread(WAVHeader, 1, sizeof(osaOpenALWAVHeader), SoundFile);

            unsigned int tmpCurrent = ftell(SoundFile);

            fseek(SoundFile, 0, SEEK_END);
            NumDataBytes = ftell(SoundFile) - tmpCurrent;
            fseek(SoundFile, tmpCurrent, SEEK_SET);
            //this is the whole file in memory.

            if (WAVHeader->wfex.wFormatTag != 1) {
                CMN_LOG_CLASS_RUN_ERROR << "OpenFile: wrong WAV format for file: " << FileName.Data << std::endl;
            }

            SoundSettings->nBytesPerSample   = WAVHeader->wfex.wBitsPerSample / 8;
            SoundSettings->nChannels         = WAVHeader->wfex.nChannels;
            SoundSettings->frequency         = WAVHeader->wfex.nSamplesPerSec;

            bytesPerWholeSample =  WAVHeader->wfex.nBlockAlign;  // WAV_Header->wfex.nChannels * WAV_Header->wfex.wBitsPerSample / 8

        }


        LengthInSec = (double)NumDataBytes / bytesPerWholeSample  / (double) SoundSettings->frequency;

        CMN_LOG_CLASS_RUN_VERBOSE << "OpenFile: file length is: "
                                  << LengthInSec.Data << " seconds" << std::endl;

        Data = new char[NumDataBytes];

        ::fread(Data, 1, NumDataBytes, SoundFile);

        CMN_LOG_CLASS_RUN_VERBOSE << "OpenFile: audioData size is: " << NumDataBytes <<std::endl;
        // << " bytes; WAV header reports Data Size: " << WAVHeader->lDataSize
        // << " bytes" << std::endl;

        ALenum tmpFormat;

        if (SoundSettings->nChannels == 1) {
            if (SoundSettings->nBytesPerSample == 1) {
                tmpFormat = AL_FORMAT_MONO8;
            } else {
                tmpFormat = AL_FORMAT_MONO16;
            }
        }
        else {
            if (SoundSettings->nBytesPerSample == 1) {
                tmpFormat = AL_FORMAT_STEREO8;
            }
            else {
                tmpFormat = AL_FORMAT_STEREO16;
            }
        }

        //Open text file containing the datatimestamps:
        std::string headerLine;
        std::string timeStampFileName = FileName.Data + std::string(".txt");
        std::ifstream timeStampFile(timeStampFileName.c_str());
        double startTime = 0;
        int    bytePos   = 0;
        double timestamp = 0;
        TimeStamps.clear();
        SamplePosInBytes.clear();

        if (timeStampFile.is_open())
        {
            getline (timeStampFile,headerLine);
            timeStampFile>>startTime>>bytePos;
            while (!timeStampFile.eof()) {
                timeStampFile>>timestamp>>bytePos;
                TimeStamps.push_back(timestamp);
                SamplePosInBytes.push_back(bytePos);
            }
        }
        else {
            CMN_LOG_CLASS_RUN_ERROR << "Can't Open file: " << timeStampFileName <<std::endl;
        }

        timeStampFile.close();

        SoundSettings->StartTime = startTime;
        StartTimeAbsolute.Data   = startTime;
        //what is the end of the data - it should use the txt.
        SoundSettings->EndTime = startTime + LengthInSec;
        CMN_LOG_CLASS_RUN_VERBOSE << std::setprecision(3) << std::fixed
                                  << "OpenFile: absolute start time: " << StartTimeAbsolute.Data << std::endl;

        alGetError();
        alGenBuffers(1, SoundBuffer);
        std::string err;

        if (CheckALError(err)) {
            CMN_LOG_CLASS_RUN_ERROR << "OpenFile: OpenAL error: " << err
                                    << " - while generating data BUFFER" << std::endl;
        }

        alBufferData(SoundBuffer[0], tmpFormat, Data, NumDataBytes, SoundSettings->frequency);
        alGenSources(1, SoundSource);

        if (CheckALError(err)) {
            CMN_LOG_CLASS_RUN_ERROR << "OpenFile: OpenAL error: " << err
                                    << "- while creating data buffer and SOURCES" << std::endl;
        }
        alSourcei(SoundSource[0], AL_BUFFER, SoundBuffer[0]);

        Seek(StartTimeAbsolute);
        RangeChangedEvent();

    }
    else {
        CMN_LOG_CLASS_RUN_VERBOSE << "OpenFile: type not Implemented" << FileName.Data << std::endl;
    }
}

std::string osaOpenAL::GetALErrorString(ALenum err)
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
    default:
        return std::string("NO_ERROR");
    };

}


bool osaOpenAL::CheckALError(std::string & error)
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


void osaOpenAL::SetVolume(const mtsDouble & volume)
{
    CMN_LOG_CLASS_RUN_VERBOSE << "SetVolume: called with " << volume.Data << std::endl;
    if (volume > 1.0)
        Volume = 1;
    else if (volume < 0)
        Volume = 0;
    else
        Volume = volume;

    alSourcef(SoundSource[0], AL_GAIN, Volume.Data);
}


bool osaOpenAL::InitOpenAL(void)
{
    Device = alcOpenDevice(NULL);
    if (Device == NULL) {
        CMN_LOG_CLASS_RUN_ERROR << "InitOpenAL: error opening device" << std::endl;
        std::string err;
        if (CheckALError(err)) {
            CMN_LOG_CLASS_INIT_ERROR << "InitOpenAL: error: " << err << std::endl;
        }
        return AL_FALSE;
    }

    Context = alcCreateContext(Device, NULL);
    if (Context == NULL) {
        alcCloseDevice (Device);
        CMN_LOG_CLASS_INIT_ERROR << "InitOpenAL: error creating context" << std::endl;
        std::string err;
        if (CheckALError(err)) {
            CMN_LOG_CLASS_INIT_ERROR << "InitOpenAL: error: " << err << std::endl;
        }
        return AL_FALSE;
    }

    if (!alcMakeContextCurrent(Context)) {
        alcDestroyContext(Context);
        alcCloseDevice(Device);
        CMN_LOG_CLASS_RUN_ERROR << "InitOpenAL: error making context current" << std::endl;
        std::string err;
        if (CheckALError(err)) {
            CMN_LOG_CLASS_INIT_ERROR << "InitOpenAL: error: " << err << std::endl;
        }
        return AL_FALSE;
    }
    return AL_TRUE;
}


bool osaOpenAL::CloseOpenAL(void)
{
    if (Device) {
        if (Context) {
            alcDestroyContext(Context);
        }
        if (!alcCloseDevice (Device)) {
            CMN_LOG_CLASS_INIT_ERROR << "CloseOpenAL: error closing device" << std::endl;
            std::string err;
            if (CheckALError(err)) {
                CMN_LOG_CLASS_INIT_ERROR << "CloseOpenAL: error: " << err << std::endl;
            }
            return AL_FALSE;
        }
    }
    else {
        return AL_FALSE;
    }

    return AL_TRUE;
}


//! \todo extend to other data taypes.
double osaOpenAL::CalcStreamVolume(int samplePos)
{

    int numberOfSamples = 1; //this is in case we want to check a sequence of samples.

    double volume = 0;
    if (SoundSettings->nBytesPerSample == 2) {
        short *buf = (short *) Data;
        int v = 0;
        for (unsigned int i = 0; i < numberOfSamples ; i++) {
            v = abs(buf[ (samplePos + i) * SoundSettings->nChannels ]); //just in case it is in stereo (get first one)
            if (volume < v) {
                volume = v;
            }
        }
        //normalize 0-100;
        volume = (volume / SHRT_MAX);
    }
    else {
        //8bit is 0-255 with 128 being zero
        int v = 0;
        for (unsigned int i = 0; i < numberOfSamples ; i++) {
            v = Data[(samplePos + i) * SoundSettings->nChannels];        //just in case it is in stereo (get first one)
            v = abs(v-127);
            if (volume < v) {
                volume = v;
            }
        }
        //normalize 0-100;
        volume = (volume / CHAR_MAX);
    }

    // std::cout<<StreamVolume.Data<<std::endl;
    return volume;
}


double osaOpenAL::CalcStreamTime(int samplePos)
{
    return (double)samplePos/(double)SoundSettings->frequency;
}


int osaOpenAL::CalcStreamPos(double time)
{
    return (int) (SoundSettings->frequency * time) ;
}

double osaOpenAL::GetStartTime() {
    return StartTimeAbsolute;

}

double osaOpenAL::GetEndTime() {

    return StartTimeAbsolute + LengthInSec;

}

void osaOpenAL::SaveClip(const std::string &filePathPrefix, double startTime, double endTime) {

    if (startTime < SoundSettings->StartTime) {
        startTime = SoundSettings->StartTime;
    }
    if (endTime > SoundSettings->EndTime) {
        endTime = SoundSettings->EndTime;
    }
    CMN_LOG_CLASS_RUN_VERBOSE <<  std::setprecision(3) << std::fixed << "Start: "<< startTime << " End: " << endTime << " diff: "<< endTime - startTime << std::endl;

    char strTime[50];
    sprintf(strTime,"_%.3f.wav", startTime);

    std::string filename = filePathPrefix + strTime;

    osaOpenALWAVHeader wavHeader;

    //Set up wav header
    sprintf(wavHeader.szRIFF, "RIFF");
    wavHeader.lRIFFSize = 0;
    sprintf(wavHeader.szWave, "WAVE");
    sprintf(wavHeader.szFmt, "fmt ");
    wavHeader.lFmtSize = sizeof(WAVEFORMATEX); //18
    wavHeader.wfex.wFormatTag = 1;
    wavHeader.wfex.nChannels = SoundSettings->nChannels;
    wavHeader.wfex.nSamplesPerSec = SoundSettings->frequency;
    wavHeader.wfex.wBitsPerSample = SoundSettings->nBytesPerSample * 8;
    wavHeader.wfex.nBlockAlign = wavHeader.wfex.nChannels * wavHeader.wfex.wBitsPerSample / 8;
    wavHeader.wfex.nAvgBytesPerSec = wavHeader.wfex.nSamplesPerSec * wavHeader.wfex.nBlockAlign;
    wavHeader.wfex.cbSize = 0;

    sprintf(wavHeader.szData, "data");
    wavHeader.lDataSize = 0;

    FILE *          saveFile;

    if ((saveFile = ::fopen(filename.c_str(), "wb")) == 0) {
        CMN_LOG_CLASS_RUN_ERROR << "Record: file name is not valid! : '" << filename << "'" <<std::endl;
        return;
    }
    else {

    }

    fwrite(WAVHeader, sizeof(osaOpenALWAVHeader), 1, saveFile);

    int fileSize = sizeof(osaOpenALWAVHeader);

    //figure out how to clip out the data (start and end samples)
    //stream pos is in local time 0 - end;
    int start = CalcStreamPos(startTime - SoundSettings->StartTime);
    int end = CalcStreamPos(endTime - SoundSettings->StartTime);

    int numOfSamples = end - start;

    CMN_LOG_CLASS_RUN_VERBOSE << "Num Of Samples: "<< numOfSamples << std::endl;

    fwrite(&Data[start * SoundSettings->nBytesPerSample], numOfSamples * SoundSettings->nBytesPerSample,1,saveFile);
    fileSize += numOfSamples * SoundSettings->nBytesPerSample;

  //  if (FType == mtsOpenALRecord::WAV) {
          wavHeader.lDataSize = fileSize;
    //}

    fclose(saveFile);

    std::ofstream    headerStream;

    OpenHeaderFile(filename,headerStream);

    WriteToHeaderFile(startTime, 0, headerStream);

    int offset = start * SoundSettings->nBytesPerSample;
    if (!TimeStamps.empty()) {

        unsigned int i = 0;
        for (i = 0; i <  TimeStamps.size(); i++) {
            if ( TimeStamps[i] > startTime )
                break;
        }

        for (i; i <  TimeStamps.size(); i++) {
            if ( TimeStamps[i] < SoundSettings->EndTime)
                WriteToHeaderFile(TimeStamps[i],SamplePosInBytes[i] - offset, headerStream);
            else
                break;
        }
    }

    CMN_LOG_CLASS_RUN_VERBOSE << "Saved Clip: '" << filename << "' size: " << fileSize/1000000 << " MBytes"<<std::endl;

    CloseHeaderFile(headerStream);
}

void osaOpenAL::OpenHeaderFile(const std::string & filename, std::ofstream & stream)
{
    stream.close();
    stream.clear();

    std::string headerFName = filename + ".txt";
    stream.open(headerFName.c_str(), std::ios::out | std::ios::app);

    if (stream.fail()) {
        CMN_LOG_CLASS_RUN_ERROR << "OpenHeaderFile: can't open file " << headerFName << std::endl;
    }
    else {
        stream.precision(4);
        stream.setf(std::ios::fixed);
        stream << "Timestamp BytesSinceStart" << std::endl;
    }
}

void osaOpenAL::CloseHeaderFile(std::ofstream & stream)
{
    stream.close();
    stream.clear();
}

void osaOpenAL::WriteToHeaderFile(const double & timestamp, const int bytes, std::ofstream &stream)
{
    stream << timestamp << " " << bytes << std::endl;
}
