/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*
  $Id: $
  
  Author(s):  Balazs Vagvolgyi
  Created on: 2013

  (C) Copyright 2006-2013 Johns Hopkins University (JHU), All Rights
  Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---

*/

#ifndef _svlFilterSource3DSensorFile_h
#define _svlFilterSource3DSensorFile_h

#include <cisstStereoVision/svlFilterSourceBase.h>

// Always include last!
#include <sawOpenNI/sawOpenNIExport.h>


// Forward declarations
class svlFilterOutput;


class CISST_EXPORT svlFilterSource3DSensorFile : public svlFilterSourceBase
{
    CMN_DECLARE_SERVICES(CMN_DYNAMIC_CREATION, CMN_LOG_ALLOW_DEFAULT);

public:
    svlFilterSource3DSensorFile();
    virtual ~svlFilterSource3DSensorFile();

    unsigned int GetWidth() const;
    unsigned int GetHeight() const;

    void SetFilepath(const std::string & filepath, bool swapcolors = false);
    void SetFilterSize(unsigned int size);
    static bool GetImageDimensions(const std::string & filepath, unsigned int & width, unsigned int & height);

protected:
    virtual int Initialize(svlSample* &syncOutput);
    virtual int Process(svlProcInfo* procInfo, svlSample* &syncOutput);
    virtual int Release();

    virtual bool LoadASIFile(const std::string & filepath, bool swapcolors = false);

private:
    svlFilterOutput* DepthOutput;
    svlFilterOutput* PointCloudOutput;
    svlSampleImageRGB*    RGBSample;
    svlSampleImageMono16* DepthSample;
    svlSampleImage3DMap*  PointCloudSample;

    unsigned int FilterSize;
    std::string Filepath;
    bool SwapColors;
    bool FilepathChanged;
};

CMN_DECLARE_SERVICES_INSTANTIATION_EXPORT(svlFilterSource3DSensorFile)

#endif // _svlFilterSource3DSensorFile_h

