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

#ifndef _osaOpenALAudioTypes_h
#define _osaOpenALAudioTypes_h

#include "al.h"
#include "alc.h"
//#include "AL/alut.h"
#include <string>

#ifdef _MSC_VER

typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int32 u_int32_t;

#else
#include <stdint.h>
#endif


// WAV
#pragma pack (push,1)
typedef struct {
    ALushort  wFormatTag;
    ALushort  nChannels;
    ALuint    nSamplesPerSec;
    ALuint    nAvgBytesPerSec;
    ALushort  nBlockAlign;
    ALushort  wBitsPerSample;
    ALushort  cbSize;
} WAVEFORMATEX;
#pragma pack (pop)


#pragma pack (push,1)
struct osaOpenALWAVHeader
{
    char            szRIFF[4];
    ALuint          lRIFFSize;  //the whole file size.
    char            szWave[4];
    char            szFmt[4];
    ALuint          lFmtSize;
    WAVEFORMATEX    wfex;
   //removed in favor of a seperate text file, ensures compatibility with standard wav files.
   // char            szTime[4];  //name of the subchunk - "abTM" - so it does not clash with other times
   // ALuint          lTimeSize;  //size of the subchunk - 8
   // double          timeStamp;  //absolute start time
    char            szData[4];
    ALuint          lDataSize;
};
#pragma pack (pop)

#pragma pack (push,1)
struct osaOpenALCISSTWAVHeader
{
    char            szRIFF[4];
    ALuint          lRIFFSize;  //the whole file size.
    char            szWave[4];
    char            szFmt[4];
    ALuint          lFmtSize;
    WAVEFORMATEX    wfex;
   //removed in favor of a seperate text file, ensures compatibility with standard wav files.
    char            szTime[4];  //name of the subchunk - "abTM" - so it does not clash with other times
    ALuint          lTimeSize;  //size of the subchunk - 8
    double          timeStamp;  //absolute start time
    char            szData[4];
    ALuint          lDataSize;
};
#pragma pack (pop)



//CAI
#pragma pack (push,1)
//Should add version number
struct osaOpenALCAIHeader
{
    ALuint  frequency;
    ALubyte nChannels;
    ALubyte nBytesPerSample;
    double  StartTime;
    double  EndTime;
};
#pragma pack (pop)


#endif //_osaOpenALAudioTypes_h
