/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*

  Author(s):  Balazs Vagvolgyi
  Created on: 2011

  (C) Copyright 2006-2011 Johns Hopkins University (JHU), All Rights
  Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---

*/

#include <sawOpenNI/svlFilterSourceKinect.h>
#include <cisstStereoVision/svlFilterOutput.h>
#include <cisstStereoVision/svlImageProcessing.h>
#include <sawOpenNI/osaOpenNI.h>


/***********************************/
/*** svlFilterSourceKinect class ***/
/***********************************/

CMN_IMPLEMENT_SERVICES_DERIVED(svlFilterSourceKinect, svlFilterSourceBase)

svlFilterSourceKinect::svlFilterSourceKinect() :
    svlFilterSourceBase(),
    Kinect(0)
{
    AddOutput("rgb", true);
    SetOutputType("rgb", svlTypeImageRGB);

    DepthOutput = AddOutput("depth", false);
    SetOutputType("depth", svlTypeImageMono16);

    PointCloudOutput = AddOutput("pointcloud", false);
    SetOutputType("pointcloud", svlTypeImage3DMap);

    const unsigned int width  = 640;
    const unsigned int height = 480;

    RGBSample        = new svlSampleImageRGB;
    DepthSample      = new svlSampleImageMono16;
    PointCloudSample = new svlSampleImage3DMap;
    RGBSample->SetSize(width, height);
    DepthSample->SetSize(width, height);
    PointCloudSample->SetSize(width, height);
}

svlFilterSourceKinect::~svlFilterSourceKinect()
{
    Release();

    if (RGBSample)        delete RGBSample;
    if (DepthSample)      delete DepthSample;
    if (PointCloudSample) delete PointCloudSample;
}

void svlFilterSourceKinect::SetKinectConfigFile(const std::string & configFile)
{
    KinectConfigFile = configFile;
}

unsigned int svlFilterSourceKinect::GetWidth() const
{
    return RGBSample->GetWidth();
}

unsigned int svlFilterSourceKinect::GetHeight() const
{
    return RGBSample->GetHeight();
}

osaOpenNI* svlFilterSourceKinect::GetKinect()
{
    return Kinect;
}

int svlFilterSourceKinect::Initialize(svlSample* &syncOutput)
{
    if (!DepthOutput || !PointCloudOutput ||
        !RGBSample || !DepthSample || !PointCloudSample) return SVL_FAIL;

    Kinect = new osaOpenNI(1);
    if (!KinectConfigFile.empty()) {
        Kinect->Configure(KinectConfigFile);
    } else {
        CMN_LOG_CLASS_INIT_ERROR << "Initialize: Kinect configuration file not set!" << std::endl;
        Release();
        return SVL_FAIL;
    }

    syncOutput = RGBSample;

    DepthOutput->SetupSample(DepthSample);
    PointCloudOutput->SetupSample(PointCloudSample);

    return SVL_OK;
}

int svlFilterSourceKinect::Process(svlProcInfo* procInfo, svlSample* &syncOutput)
{
    syncOutput = RGBSample;

    _OnSingleThread(procInfo)
    {
        Kinect->Update(WAIT_AND_UPDATE_ALL);

        // Acquire color and depth images
        Kinect->GetRGBImage(RGBSample->GetMatrixRef());
        Kinect->GetDepthImageRaw(DepthSample->GetMatrixRef());

        vctDynamicMatrixRef<vctFloat3> pointcloud(GetHeight(), GetWidth(), // rows, columns
                                                  GetWidth(), 1,           // rowstride, columnstride
                                                  reinterpret_cast<vctFloat3*>(PointCloudSample->GetPointer()));
        Kinect->GetRangeData(pointcloud);

        // Post-process captured data
        svlImageProcessing::SwapColorChannels(RGBSample, 0, RGBSample, 0);

        // Push depth image to async output
        DepthSample->SetTimestamp(RGBSample->GetTimestamp());
        DepthOutput->PushSample(DepthSample);

        // Push point cloud to async output
        PointCloudSample->SetTimestamp(RGBSample->GetTimestamp());
        PointCloudOutput->PushSample(PointCloudSample);
    }

    return SVL_OK;
}

int svlFilterSourceKinect::Release()
{
    if (Kinect) {
        Kinect->CleanupExit();
        delete Kinect;
        Kinect = 0;
    }

    return SVL_OK;
}

