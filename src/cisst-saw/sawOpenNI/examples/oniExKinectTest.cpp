/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*

  Author(s):  Kelleher Guerin and Simon Leonard
  Created on: 2008

  (C) Copyright 2006-2008 Johns Hopkins University (JHU), All Rights
  Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---

*/

#include <sawOpenNI/osaOpenNI.h>
#include <cisstCommon/cmnLogger.h>
#include <cisstCommon/cmnGetChar.h>
#include <cisstCommon/cmnPath.h>
#include <cisstVector/vctDynamicMatrix.h>
#include <cisstOSAbstraction/osaSleep.h>

int main(int argc, char** argv){

	cmnLogger::SetMask(CMN_LOG_ALLOW_ALL);
	cmnLogger::SetMaskFunction(CMN_LOG_ALLOW_ALL);
	cmnLogger::SetMaskDefaultLog(CMN_LOG_ALLOW_ALL);

    int numusers = 0;
    if (argc == 2) {
        sscanf(argv[1], "%d", &numusers);
    }


    cmnPath path;
    path.Add(".");
    std::string configFile = path.Find("SamplesConfig.xml");
    if (configFile == "") {
        std::cerr << "can't find file \"SamplesConfig.xml\" in path: " << path << std::endl;
        exit (-1);
    }
	osaOpenNI kinect(numusers);
    kinect.Configure(configFile);
    if (0 < numusers) {
        kinect.InitSkeletons();
    }

	while (true) {
        // Wait and Update All
        kinect.Update(WAIT_AND_UPDATE_ALL);
        if (0 < numusers) {
            kinect.UpdateUserSkeletons();
        }

        {
            vctDynamicMatrix<unsigned char> rgb;
            if (kinect.GetRGBImage(rgb) != osaOpenNI::ESUCCESS) {
                CMN_LOG_RUN_ERROR << "Failed to get RGB image" << std::endl;
                return -1;
            }
            std::ofstream ofs("rgb");
            for (size_t r=0; r<rgb.rows(); r++) {
                for (size_t c=0; c<rgb.cols(); c++) {
                    ofs << (int)rgb[r][c] << " ";
                }
                ofs << std::endl;
            }
            ofs.close();
        }

        {
            vctDynamicMatrix<double> depth;
            if (kinect.GetDepthImageRaw(depth) != osaOpenNI::ESUCCESS) {
                CMN_LOG_RUN_ERROR << "Failed to get RGB image" << std::endl;
                return -1;
            }
            std::ofstream ofs("depth");
            ofs << depth;
            ofs.close();
        }

        {
            vctDynamicMatrix<double> range;
            std::vector< vctFixedSizeVector<unsigned short, 2> > pixels;
            if (kinect.GetRangeData(range, pixels) != osaOpenNI::ESUCCESS) {
                CMN_LOG_RUN_ERROR << "Failed to get RGB image" << std::endl;
                return -1;
            }
            std::ofstream ofs("range");
            ofs << range;
            ofs.close();
        }

        if (0 < numusers) {
            std::vector<osaOpenNISkeleton*> skeletons = kinect.UpdateAndGetUserSkeletons();
        }
        std::cerr << "*";
	}

	return 0;
}
