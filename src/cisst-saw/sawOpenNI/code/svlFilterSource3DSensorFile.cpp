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

#include <sawOpenNI/svlFilterSource3DSensorFile.h>
#include <cisstStereoVision/svlFilterOutput.h>
#include <cisstStereoVision/svlImageProcessing.h>


/*****************************************/
/*** svlFilterSource3DSensorFile class ***/
/*****************************************/

CMN_IMPLEMENT_SERVICES_DERIVED(svlFilterSource3DSensorFile, svlFilterSourceBase)

svlFilterSource3DSensorFile::svlFilterSource3DSensorFile() :
    svlFilterSourceBase(),
    FilterSize(6)
{
    AddOutput("rgb", true);
    SetOutputType("rgb", svlTypeImageRGB);

    DepthOutput = AddOutput("depth", false);
    SetOutputType("depth", svlTypeImageMono16);

    PointCloudOutput = AddOutput("pointcloud", false);
    SetOutputType("pointcloud", svlTypeImage3DMap);

    RGBSample        = new svlSampleImageRGB;
    DepthSample      = new svlSampleImageMono16;
    PointCloudSample = new svlSampleImage3DMap;
}

svlFilterSource3DSensorFile::~svlFilterSource3DSensorFile()
{
    Release();

    if (RGBSample)        delete RGBSample;
    if (DepthSample)      delete DepthSample;
    if (PointCloudSample) delete PointCloudSample;
}

unsigned int svlFilterSource3DSensorFile::GetWidth() const
{
    return RGBSample->GetWidth();
}

unsigned int svlFilterSource3DSensorFile::GetHeight() const
{
    return RGBSample->GetHeight();
}

void svlFilterSource3DSensorFile::SetFilepath(const std::string & filepath, bool swapcolors)
{
    Filepath = filepath;
    SwapColors = swapcolors;
    FilepathChanged = true;
}

void svlFilterSource3DSensorFile::SetFilterSize(unsigned int size)
{
    FilterSize = size;
}

bool svlFilterSource3DSensorFile::GetImageDimensions(const std::string & filepath, unsigned int & width, unsigned int & height)
{
    svlFilterSource3DSensorFile filt;
    if (!filt.LoadASIFile(filepath)) return false;
    width  = filt.GetWidth();
    height = filt.GetHeight();
    return true;
}

int svlFilterSource3DSensorFile::Initialize(svlSample* &syncOutput)
{
    if (!DepthOutput || !PointCloudOutput ||
        !RGBSample || !DepthSample || !PointCloudSample) return SVL_FAIL;

    syncOutput = RGBSample;

    if (!LoadASIFile(Filepath, SwapColors)) return SVL_FAIL;
    FilepathChanged = false;

    DepthOutput->SetupSample(DepthSample);
    PointCloudOutput->SetupSample(PointCloudSample);

    return SVL_OK;
}

int svlFilterSource3DSensorFile::Process(svlProcInfo* procInfo, svlSample* &syncOutput)
{
    syncOutput = RGBSample;

    _OnSingleThread(procInfo)
    {
        if (FilepathChanged) {
            if (!LoadASIFile(Filepath, SwapColors)) return SVL_FAIL;
            FilepathChanged = false;
        }

        // Push depth image to async output
        DepthSample->SetTimestamp(RGBSample->GetTimestamp());
        DepthOutput->PushSample(DepthSample);

        // Push point cloud to async output
        PointCloudSample->SetTimestamp(RGBSample->GetTimestamp());
        PointCloudOutput->PushSample(PointCloudSample);
    }

    return SVL_OK;
}

int svlFilterSource3DSensorFile::Release()
{
    // Release Kinect stuff
    return SVL_OK;
}

bool svlFilterSource3DSensorFile::LoadASIFile(const std::string & filepath, bool swapcolors)
{
    std::ifstream ifs(filepath.c_str(), std::ios::in|std::ios::binary);

    if (ifs.is_open()) {

        unsigned int version, frame_id, data_size;
        unsigned char has_depth;
        char camera_name[16];

        ifs.read((char*)&version,   sizeof(version));
        ifs.read((char*)&frame_id,  sizeof(frame_id));
        ifs.read((char*)&has_depth, sizeof(has_depth));

        switch (version) {
            case 2:
            {
                unsigned int tmp;
                ifs.read((char*)&tmp, sizeof(tmp));
                ifs.read((char*)&tmp, sizeof(tmp));
#if 0
                ifs.read((char*)&tmp, sizeof(tmp));
                ifs.read((char*)&tmp, sizeof(tmp));
#endif
            }
            break;

            case 3:
            {
                unsigned int tmp;
                ifs.read((char*)&tmp, sizeof(tmp));
                ifs.read((char*)&tmp, sizeof(tmp));
                ifs.read((char*)&tmp, sizeof(tmp));
                ifs.read((char*)&tmp, sizeof(tmp));
            }
            break;
        }

        ifs.read((char*)&camera_name, sizeof(camera_name));

        std::cout << "Image: (ver=" << version << ", frame#=" << frame_id << ", depth=" << (int)has_depth << ", camera=" << camera_name;


        // Read intensity image (JPEG)
        ifs.read((char*)&data_size, sizeof(data_size));

        std::cout << ", data_size=" << data_size << ")" << std::endl;

        unsigned char* buffer = new unsigned char[data_size];
        ifs.read((char*)buffer, data_size);

        cv::Mat intensity = cv::imdecode(cv::Mat(std::vector<unsigned char>(buffer, buffer + data_size)), 1);

        delete [] buffer;

        RGBSample->SetSize(intensity.cols, intensity.rows);
        memcpy(RGBSample->GetUCharPointer(), intensity.data, RGBSample->GetDataSize());

        if (swapcolors) svlImageProcessing::SwapColorChannels(RGBSample, 0, RGBSample, 0);

        // Read depth image (PNG)
        if (has_depth) {
            ifs.read((char*)&data_size, sizeof(data_size));

            std::cout << "Depth map: (data_size=" << data_size << ")" << std::endl;

            buffer = new unsigned char[data_size];
            ifs.read((char*)buffer, data_size);

            cv::Mat depth = cv::imdecode(cv::Mat(std::vector<unsigned char>(buffer, buffer + data_size)), 1);

            delete [] buffer;

            DepthSample->SetSize(depth.cols, depth.rows);
            PointCloudSample->SetSize(depth.cols, depth.rows);

            unsigned char*  inbuf  = depth.data;
            unsigned short* outbuf = reinterpret_cast<unsigned short*>(DepthSample->GetUCharPointer());
            unsigned short u16;

            // Convert to unsigned short
            for (int r = 0; r < depth.rows; r++) {
                for (int c = 0; c < depth.cols; c++) {
                    u16 = inbuf[1];
                    u16 <<= 8;
                    u16 += inbuf[0];

                    if (u16 == 32512) {
                        *outbuf = 0;
                    }
                    else {
                        *outbuf = u16;
                    }

                    inbuf += 3;
                    outbuf ++;
                }
            }

            double fl, bl, cx, cy;

            ifs.read((char*)&fl, sizeof(fl));
            ifs.read((char*)&bl, sizeof(bl));
            ifs.read((char*)&cx, sizeof(cx));
            ifs.read((char*)&cy, sizeof(cy));

            // Bug fixes
            if (bl == 0.0) bl = 0.1;
            if (cx == 0)   cx = 320;
            if (cy == 320) cy = 240;

            std::cout << "Camera parameters: (fl=" << fl << ", bl=" << bl << ", cx=" << cx << ", cy=" << cy << ")" << std::endl;


            // Generate 3D pointcloud
            cv::Mat depth_f32;
            DepthSample->CvMatRef().convertTo(depth_f32, CV_32FC1);

            cv::Mat blf = depth_f32.clone();
            if (FilterSize > 0) {
                try {
                    cv::bilateralFilter(depth_f32, blf, -1, 500, FilterSize);
                }
                catch (...) {
                    std::cout << "Bilateral filtering failed" << std::endl;
                }
            }
            else {
                depth_f32.copyTo(blf);
            }

            float* pclbuf_filt = reinterpret_cast<float*>(blf.data);
            float* pclbuf = reinterpret_cast<float*>(PointCloudSample->GetUCharPointer());
            outbuf = reinterpret_cast<unsigned short*>(DepthSample->GetUCharPointer());

            float dpth_flt;
            double z;
            int dpth;

            for (int r = 0; r < depth.rows; r ++) {
                for (int c = 0; c < depth.cols; c ++) {
                    dpth = outbuf[0];
                    dpth_flt = pclbuf_filt[0];

                    if (0 < dpth && 0.1 < dpth_flt && dpth < 0x7f7f) {
                        z = fl * 256.0 * bl / dpth_flt;

                        if (0.1 < z) {
                            pclbuf[0] = (c - cx) * 256.0 * bl / dpth_flt;
                            pclbuf[1] = (r - cy) * 256.0 * bl / dpth_flt;
                            pclbuf[2] = z;
                        }
                        else {
                            pclbuf[0] = pclbuf[1] = pclbuf[2] = 0.0f;
                        }
                    }
                    else {
                        pclbuf[0] = pclbuf[1] = pclbuf[2] = 0.0f;
                    }

                    pclbuf += 3;
                    outbuf ++;
                    pclbuf_filt ++;
                }
            }
        }

        return true;
    }

    std::cerr << "Failed to open " << filepath << std::endl;

    return false;
}

