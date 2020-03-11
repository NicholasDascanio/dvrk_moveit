/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*
  $Id: svlFilterPlaneSegmentation.h $

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

#ifndef _svlFilterPlaneSegmentation_h
#define _svlFilterPlaneSegmentation_h

#include <cisstStereoVision/svlFilterBase.h>
#include <sawOpenNI/oniPlaneSegmentation.h>

// Always include last!
#include <sawOpenNI/sawOpenNIExport.h>


// Forward declarations
class svlFilterInput;
class svlFilterOutput;


class CISST_EXPORT svlFilterPlaneSegmentation : public svlFilterBase
{
    CMN_DECLARE_SERVICES(CMN_DYNAMIC_CREATION, CMN_LOG_ALLOW_DEFAULT);

public:
    svlFilterPlaneSegmentation();
    virtual ~svlFilterPlaneSegmentation();

    virtual void PrintObjectPoints();

protected:

    virtual int Initialize(svlSample* syncInput, svlSample* &syncOutput);
    virtual int Process(svlProcInfo* procInfo, svlSample* syncInput, svlSample* &syncOutput);

    virtual void PrintObjects(std::vector< std::vector<oniRGBCRXYZ> > & objects);

private:
    bool PrintFlag;

    svlFilterInput*  PointCloudInput;
    svlFilterOutput* VisualizedImageOutput;
    svlFilterOutput* PlaneDistancesOutput;
    svlFilterOutput* PlaneObjectsOutput;
    svlFilterOutput* UVHistogramOutput;

    svlSampleImageRGB*    VisualizedImage;
    svlSampleImageMono16* PlaneDistances;
    svlSampleBlobs*       PlaneObjects;
    svlSampleImageRGB*    UVHistogram;

public:
    oniPlaneSegmentation Segmentation;
};

CMN_DECLARE_SERVICES_INSTANTIATION_EXPORT(svlFilterPlaneSegmentation)

#endif // _svlFilterPlaneSegmentation_h

