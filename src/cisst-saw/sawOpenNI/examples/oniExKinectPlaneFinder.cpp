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

#include <cisstStereoVision.h>
#include <cisstCommon/cmnGetChar.h>
#include <cisstCommon/cmnPath.h>

#define SIMULATE_SENSOR 1

#if SIMULATE_SENSOR == 0
    #include <sawOpenNI/svlFilterSourceKinect.h>
    #include <sawOpenNI/osaOpenNI.h>
#else
    #include <sawOpenNI/svlFilterSource3DSensorFile.h>
#endif

#include <sawOpenNI/svlFilterPlaneSegmentation.h>


std::vector<std::string> ReadFileList(const std::string & filepath)
{
    std::vector<std::string> filenames;
    
    std::ifstream list(filepath.c_str());
    char line[512];

    std::cout << ". Loading file list:" << std::endl;
    do {
        line[0] = 0;
        list.getline(line, 512);
        if (line[0] && !list.eof()) {
            filenames.push_back(line);
            std::cout << "    " << line << std::endl;
        }
    } while (!list.eof());
    std::cout << std::endl;

    return filenames;
}

int main(int argc, char** argv)
{
    svlInitialize();

    svlStreamManager stream;

#if SIMULATE_SENSOR == 0
    svlFilterSourceKinect sensor;
#else
    svlFilterSource3DSensorFile sensor;
#endif

    svlFilterImageChannelSwapper swapper1, swapper2, swapper3, swapper4;
    svlFilterImageOverlay overlay;
    svlFilterImageWindow window1, window2, window3, window4, window5;
    svlFilterStreamTypeConverter to_rgb1(svlTypeImageMono16, svlTypeImageRGB);
    svlFilterStreamTypeConverter to_rgb2(svlTypeImageMono16, svlTypeImageRGB);

    svlFilterPlaneSegmentation segmentation;

#if SIMULATE_SENSOR == 0
    // Set Kinect configuration file
    cmnPath path;
    path.Add(".");
    std::string configFile = path.Find("SamplesConfig.xml");
    if (configFile == "") {
        std::cerr << "can't find file \"SamplesConfig.xml\" in path: " << path << std::endl;
        exit (-1);
    }
    sensor.SetKinectConfigFile(configFile);
#else
    int file_idx = 0;
    std::vector<std::string> filelist;
    if (argc > 1) {
        filelist = ReadFileList(argv[1]);
    }
    else {
        filelist.push_back("/Users/vagvoba/Code/jhu-asi/data/jhu-asi_repositotory/m1/saved-frame-25837016.bin");
    }
    sensor.SetFilepath(filelist[file_idx]);
#endif

    // Setup Mono16 to RGB converter
    to_rgb1.SetMono16ShiftDown(6);
    to_rgb2.SetMono16ShiftDown(8);

    overlay.AddInputBlobs("blobs");
    svlOverlayBlobs ovrl_blobs1(0, true, "blobs", 0);
    overlay.AddOverlay(ovrl_blobs1);
    overlay.AddQueuedItems();

    // Setup windows
    window1.SetTitle("Segmented Image");
    window2.SetTitle("Depth Image");
    window3.SetTitle("Elevation Map");
    window4.SetTitle("UV Histogram");
    window5.SetTitle("RGB Image");

#if 1
    // Join branches
    unsigned int width, height;
    svlFilterSource3DSensorFile::GetImageDimensions(filelist[file_idx], width, height);
    svlFilterImageBorder border;
    border.SetBorders(0, 0, width * 2, 0);
    svlFilterImageOverlay joiner;
    joiner.AddInputImage("elevation");
    joiner.AddInputImage("segmented");
    svlOverlayImage ovrl_elevation(0, true, "elevation", 0, vctInt2(width,     0), 255);
    svlOverlayImage ovrl_segmented(0, true, "segmented", 0, vctInt2(width * 2, 0), 255);
    joiner.AddOverlay(ovrl_elevation);
    joiner.AddOverlay(ovrl_segmented);
    joiner.AddQueuedItems();
    svlFilterImageFileWriter filewriter;
    filewriter.SetFilePath("result_", "jpg");
    filewriter.SetCompression(92);
    filewriter.Pause();
#endif

    int ret;

    // Chain filters to trunk
    stream.SetSourceFilter(&sensor);

    ret = sensor.GetOutput("rgb")->Connect(swapper1.GetInput());
    std::cerr << ". Connection from 'sensor:rgb' to 'swapper1' " << (ret == 0 ? "successful" : "failed") << std::endl;

    ret = swapper1.GetOutput()->Connect(segmentation.GetInput("rgb"));
    std::cerr << ". Connection from 'swapper1' to 'segmentation:rgb' " << (ret == 0 ? "successful" : "failed") << std::endl;
    ret = sensor.GetOutput("pointcloud")->Connect(segmentation.GetInput("pointcloud"));
    std::cerr << ". Connection from 'sensor:pointcloud' to 'segmentation:pointcloud' " << (ret == 0 ? "successful" : "failed") << std::endl;
    ret = sensor.GetOutput("depth")->Connect(to_rgb1.GetInput());
    std::cerr << ". Connection from 'sensor:depth' to 'to_rgb1' " << (ret == 0 ? "successful" : "failed") << std::endl;

    ret = segmentation.GetOutput("visualized")->Connect(overlay.GetInput());
    std::cerr << ". Connection from 'segmentation:visualized' to 'overlay' " << (ret == 0 ? "successful" : "failed") << std::endl;
    ret = segmentation.GetOutput("planedistances")->Connect(to_rgb2.GetInput());
    std::cerr << ". Connection from 'segmentation:planedistances' to 'to_rgb2' " << (ret == 0 ? "successful" : "failed") << std::endl;
    ret = segmentation.GetOutput("planeobjects")->Connect(overlay.GetInput("blobs"));
    std::cerr << ". Connection from 'segmentation:planeobjects' to 'overlay:blobs' " << (ret == 0 ? "successful" : "failed") << std::endl;
    ret = segmentation.GetOutput("uv_histogram")->Connect(swapper2.GetInput());
    std::cerr << ". Connection from 'segmentation:uv_histogram' to 'swapper2' " << (ret == 0 ? "successful" : "failed") << std::endl;
    ret = segmentation.GetOutput("rgb")->Connect(swapper3.GetInput());
    std::cerr << ". Connection from 'segmentation:rgb' to 'swapper3' " << (ret == 0 ? "successful" : "failed") << std::endl;

    ret = swapper2.GetOutput()->Connect(window4.GetInput());
    std::cerr << ". Connection from 'swapper2' to 'window4' " << (ret == 0 ? "successful" : "failed") << std::endl;
    ret = swapper3.GetOutput()->Connect(window5.GetInput());
    std::cerr << ". Connection from 'swapper3' to 'window5' " << (ret == 0 ? "successful" : "failed") << std::endl;

    ret = overlay.GetOutput()->Connect(swapper4.GetInput());
    std::cerr << ". Connection from 'overlay' to 'swapper4' " << (ret == 0 ? "successful" : "failed") << std::endl;
    ret = swapper4.GetOutput()->Connect(window1.GetInput());
    std::cerr << ". Connection from 'swapper4' to 'window1' " << (ret == 0 ? "successful" : "failed") << std::endl;

    ret = to_rgb1.GetOutput()->Connect(window2.GetInput());
    std::cerr << ". Connection from 'to_rgb1' to 'window2' " << (ret == 0 ? "successful" : "failed") << std::endl;
    ret = to_rgb2.GetOutput()->Connect(window3.GetInput());
    std::cerr << ". Connection from 'to_rgb2' to 'window3' " << (ret == 0 ? "successful" : "failed") << std::endl;

#if 1
    ret = window5.GetOutput()->Connect(border.GetInput());
    std::cerr << ". Connection from 'window5' to 'border' " << (ret == 0 ? "successful" : "failed") << std::endl;
    ret = border.GetOutput()->Connect(joiner.GetInput());
    std::cerr << ". Connection from 'border' to 'joiner' " << (ret == 0 ? "successful" : "failed") << std::endl;
    ret = window3.GetOutput()->Connect(joiner.GetInput("elevation"));
    std::cerr << ". Connection from 'window3' to 'joiner:elevation' " << (ret == 0 ? "successful" : "failed") << std::endl;
    ret = window1.GetOutput()->Connect(joiner.GetInput("segmented"));
    std::cerr << ". Connection from 'window1' to 'joiner:segmented' " << (ret == 0 ? "successful" : "failed") << std::endl;
    ret = joiner.GetOutput()->Connect(filewriter.GetInput());
    std::cerr << ". Connection from 'joiner' to 'filewriter' " << (ret == 0 ? "successful" : "failed") << std::endl;
#endif

    // Initialize and start stream
    if (stream.Play() == SVL_OK) {
        int ch;
        unsigned int keycounter = 0;

        do {
            if ((keycounter % 20) == 0) {
                std::cerr << std::endl << "------------------------------------------" << std::endl;
                std::cerr << "GradientRadius [pixels]     = " << segmentation.Segmentation.GetGradientRadius() << std::endl;
                std::cerr << "PeakRadius [pixels]         = " << (int)segmentation.Segmentation.GetPeakRadius() << std::endl;
                std::cerr << "PlaneDistanceThreshold [mm] = " << segmentation.Segmentation.GetPlaneDistanceThreshold() << std::endl;
                std::cerr << "ColorMatchWeight            = " << std::fixed << std::setprecision(2) << segmentation.Segmentation.GetColorMatchWeight() << std::endl;
                std::cerr << "MinObjectArea [pixels]      = " << segmentation.Segmentation.GetMinObjectArea() << std::endl;
                std::cerr << "MergeThreshold              = " << segmentation.Segmentation.GetMergeThreshold() << std::endl << std::endl;
                std::cerr << "PlaneID                     = " << segmentation.Segmentation.GetPlaneID() << std::endl << std::endl;
                std::cerr << "Keyboard commands in command window:" << std::endl;
                std::cerr << "  'p'   - Print object points" << std::endl;
                std::cerr << "  '1'   - GradientRadius --" << std::endl;
                std::cerr << "  '2'   - GradientRadius ++" << std::endl;
                std::cerr << "  '3'   - PeakRadius --" << std::endl;
                std::cerr << "  '4'   - PeakRadius ++" << std::endl;
                std::cerr << "  '5'   - PlaneDistanceThreshold --" << std::endl;
                std::cerr << "  '6'   - PlaneDistanceThreshold ++" << std::endl;
                std::cerr << "  '7'   - ColorMatchWeight --" << std::endl;
                std::cerr << "  '8'   - ColorMatchWeight ++" << std::endl;
                std::cerr << "  '9'   - MinObjectArea --" << std::endl;
                std::cerr << "  '0'   - MinObjectArea ++" << std::endl;
                std::cerr << "  '-'   - MergeThreshold --" << std::endl;
                std::cerr << "  '='   - MergeThreshold ++" << std::endl;
                std::cerr << "  SPACE - Switch to next plane" << std::endl;
                std::cerr << "  'q'   - Quit" << std::endl;
                std::cerr << "------------------------------------------" << std::endl << std::endl;
            }
            keycounter ++;

            ch = cmnGetChar();
            switch (ch) {
                case 'p':
                    segmentation.PrintObjectPoints();
                break;

                case '1':
                    segmentation.Segmentation.SetGradientRadius(segmentation.Segmentation.GetGradientRadius() - 1);
                    std::cerr << "  GradientRadius [pixels] = " << segmentation.Segmentation.GetGradientRadius() << std::endl;
                break;

                case '2':
                    segmentation.Segmentation.SetGradientRadius(segmentation.Segmentation.GetGradientRadius() + 1);
                    std::cerr << "  GradientRadius [pixels] = " << segmentation.Segmentation.GetGradientRadius() << std::endl;
                break;

                case '3':
                    segmentation.Segmentation.SetPeakRadius(segmentation.Segmentation.GetPeakRadius() - 1);
                    std::cerr << "  PeakRadius [pixels] = " << (int)segmentation.Segmentation.GetPeakRadius() << std::endl;
                break;

                case '4':
                    segmentation.Segmentation.SetPeakRadius(segmentation.Segmentation.GetPeakRadius() + 1);
                    std::cerr << "  PeakRadius [pixels] = " << (int)segmentation.Segmentation.GetPeakRadius() << std::endl;
                break;

                case '5':
                    segmentation.Segmentation.SetPlaneDistanceThreshold(segmentation.Segmentation.GetPlaneDistanceThreshold() - 1.0);
                    std::cerr << "  PlaneDistanceThreshold [mm] = " << (int)segmentation.Segmentation.GetPlaneDistanceThreshold() << std::endl;
                break;

                case '6':
                    segmentation.Segmentation.SetPlaneDistanceThreshold(segmentation.Segmentation.GetPlaneDistanceThreshold() + 1.0);
                    std::cerr << "  PlaneDistanceThreshold [mm] = " << (int)segmentation.Segmentation.GetPlaneDistanceThreshold() << std::endl;
                break;

                case '7':
                    segmentation.Segmentation.SetColorMatchWeight(segmentation.Segmentation.GetColorMatchWeight() - 0.05);
                    std::cerr << "  ColorMatchWeight = " << std::fixed << std::setprecision(2) << segmentation.Segmentation.GetColorMatchWeight() << std::endl;
                break;

                case '8':
                    segmentation.Segmentation.SetColorMatchWeight(segmentation.Segmentation.GetColorMatchWeight() + 0.05);
                    std::cerr << "  ColorMatchWeight = " << std::fixed << std::setprecision(2) << segmentation.Segmentation.GetColorMatchWeight() << std::endl;
                break;

                case '9':
                    segmentation.Segmentation.SetMinObjectArea(segmentation.Segmentation.GetMinObjectArea() - 100);
                    std::cerr << "  MinObjectArea [pixels] = " << segmentation.Segmentation.GetMinObjectArea() << std::endl;
                break;

                case '0':
                    segmentation.Segmentation.SetMinObjectArea(segmentation.Segmentation.GetMinObjectArea() + 100);
                    std::cerr << "  MinObjectArea [pixels] = " << segmentation.Segmentation.GetMinObjectArea() << std::endl;
                break;

                case '-':
                    segmentation.Segmentation.SetMergeThreshold(segmentation.Segmentation.GetMergeThreshold() - 0.01f);
                    std::cerr << "  MergeThreshold = " << segmentation.Segmentation.GetMergeThreshold() << std::endl;
                break;

                case '=':
                    segmentation.Segmentation.SetMergeThreshold(segmentation.Segmentation.GetMergeThreshold() + 0.01f);
                    std::cerr << "  MergeThreshold = " << segmentation.Segmentation.GetMergeThreshold() << std::endl;
                break;

                case ' ':
                    segmentation.Segmentation.SetPlaneID(segmentation.Segmentation.GetPlaneID() + 1);
                    std::cerr << "  PlaneID = " << segmentation.Segmentation.GetPlaneID() << std::endl;
                break;

                case 'n':
                    file_idx --;
                    if (file_idx < 0) file_idx = (int)filelist.size() - 1;
                    sensor.SetFilepath(filelist[file_idx]);
                    std::cerr << ".  Loading file: " << filelist[file_idx] << std::endl;
                break;

                case 'm':
                    file_idx ++;
                    if (file_idx >= (int)filelist.size()) file_idx = 0;
                    sensor.SetFilepath(filelist[file_idx]);
                    std::cerr << ".  Loading file: " << filelist[file_idx] << std::endl;
                break;

#if 1
                case 's':
                    filewriter.Record(1);
                break;
#endif
            }
        } while (ch != 'q' && ch != 'Q');

        std::cout << "Quitting." << std::endl;
    }
    else {
        std::cout << "Error... Quitting." << std::endl;
    }

    // Safely stopping and deconstructing stream before de-allocation
    stream.Release();
    stream.DisconnectAll();

    return 1;
}

