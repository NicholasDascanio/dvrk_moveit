/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*
  $Id: svlFilterPlaneSegmentation.cpp $

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

#include <sawOpenNI/svlFilterPlaneSegmentation.h>
#include <cisstStereoVision/svlFilterInput.h>
#include <cisstStereoVision/svlFilterOutput.h>


/****************************************/
/*** svlFilterPlaneSegmentation class ***/
/****************************************/

CMN_IMPLEMENT_SERVICES_DERIVED(svlFilterPlaneSegmentation, svlFilterBase)

svlFilterPlaneSegmentation::svlFilterPlaneSegmentation() :
    svlFilterBase(),
    PrintFlag(false)
{
    AddInput("rgb", true);
    AddInputType("rgb", svlTypeImageRGB);
    AddOutput("rgb", true);
    SetOutputType("rgb", svlTypeImageRGB);

    PointCloudInput = AddInput("pointcloud", false);
    AddInputType("pointcloud", svlTypeImage3DMap);

    VisualizedImage = new svlSampleImageRGB;
    PlaneDistances  = new svlSampleImageMono16;
    PlaneObjects    = new svlSampleBlobs;
    UVHistogram     = new svlSampleImageRGB;

    VisualizedImageOutput = AddOutput("visualized",      false);
    PlaneDistancesOutput  = AddOutput("planedistances",  false);
    PlaneObjectsOutput    = AddOutput("planeobjects",    false);
    UVHistogramOutput     = AddOutput("uv_histogram",    false);
    SetOutputType("visualized",     svlTypeImageRGB);
    SetOutputType("planedistances", svlTypeImageMono16);
    SetOutputType("planeobjects",   svlTypeBlobs);
    SetOutputType("uv_histogram",   svlTypeImageRGB);
}

svlFilterPlaneSegmentation::~svlFilterPlaneSegmentation()
{
    Release();

    if (VisualizedImage) delete VisualizedImage;
    if (PlaneDistances)  delete PlaneDistances;
    if (PlaneObjects)    delete PlaneObjects;
    if (UVHistogram)     delete UVHistogram;
}

void svlFilterPlaneSegmentation::PrintObjectPoints()
{
    PrintFlag = true;
}

int svlFilterPlaneSegmentation::Initialize(svlSample* syncInput, svlSample* &syncOutput)
{
    syncOutput = syncInput;

    VisualizedImage->SetSize(syncInput);
    PlaneDistances->SetSize(syncInput);
    PlaneObjects->SetChannelCount(1);
    PlaneObjects->SetBufferSize(1000);
    UVHistogram->SetSize(256, 256);

    VisualizedImageOutput->SetupSample(VisualizedImage);
    PlaneDistancesOutput->SetupSample(PlaneDistances);
    PlaneObjectsOutput->SetupSample(PlaneObjects);
    UVHistogramOutput->SetupSample(UVHistogram);

    return SVL_OK;
}

int svlFilterPlaneSegmentation::Process(svlProcInfo* procInfo, svlSample* syncInput, svlSample* &syncOutput)
{
    syncOutput = syncInput;

    _OnSingleThread(procInfo)
    {
        // Pull point cloud from async input
        svlSampleImage3DMap* pointcloud_sample = dynamic_cast<svlSampleImage3DMap*>(PointCloudInput->PullSample(true));
        if (!pointcloud_sample) return SVL_FAIL;

        Segmentation.Process(dynamic_cast<svlSampleImageRGB*>(syncInput),
                             pointcloud_sample,
                             VisualizedImage,
                             PlaneDistances,
                             PlaneObjects);

        // Use these methods to retrieve detected planes:
        //
        //   void GetPlaneIDs(vctDynamicVector<unsigned int> & id_vector) const;
        //   void GetPlaneIDMap(svlSampleImageMono32 & id_map) const;
        //   const oniPlane& GetPlane(unsigned int plane_id) const;
        //   int GetPlaneCopy(oniPlane & plane, unsigned int plane_id) const;
        //

        Segmentation.GetUVHistogram(UVHistogram);

        if (PrintFlag) {
            // Extract object pointclouds in std::vector format
            std::vector< std::vector<oniRGBCRXYZ> > objects;
            Segmentation.GetObjectVector(objects, dynamic_cast<svlSampleImageRGB*>(syncInput), pointcloud_sample, PlaneObjects);

            std::cout << std::endl;
            PrintObjects(objects);
            std::cout << std::endl;
            PrintFlag = false;
        }

        // Push samples to async outputs
        VisualizedImage->SetTimestamp(syncInput->GetTimestamp());
        PlaneDistances->SetTimestamp(syncInput->GetTimestamp());
        PlaneObjects->SetTimestamp(syncInput->GetTimestamp());
        UVHistogram->SetTimestamp(syncInput->GetTimestamp());

        VisualizedImageOutput->PushSample(VisualizedImage);
        PlaneDistancesOutput->PushSample(PlaneDistances);
        PlaneObjectsOutput->PushSample(PlaneObjects);
        UVHistogramOutput->PushSample(UVHistogram);
    }

    return SVL_OK;
}

void svlFilterPlaneSegmentation::PrintObjects(std::vector< std::vector<oniRGBCRXYZ> > & objects)
{
    for (unsigned int o = 0; o < objects.size(); o ++) {
        std::cout << "Object " << o << ":" << std::endl;
        for (unsigned int p = 0; p < objects[o].size(); p ++) {
            oniRGBCRXYZ& point = objects[o][p];
            std::cout << "  point " << p << ": rgb=("
                      << static_cast<int>(point.r) << ", " << static_cast<int>(point.g) << ", " << static_cast<int>(point.b) << ") cr=("
                      << point.col << ", " << point.row << ") xyz=("
                      << point.x << ", " << point.y << ", " << point.z << ")" << std::endl;
        }
    }
}

