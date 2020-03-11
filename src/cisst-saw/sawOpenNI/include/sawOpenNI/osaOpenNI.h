/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*

  Author(s): Kelleher Guerin, Simon Leonard
  Created on: 2011

  (C) Copyright 2011 Johns Hopkins University (JHU), All Rights
  Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---

*/


#ifndef _osaOpenNI_h
#define _osaOpenNI_h

#include <sawOpenNI/osaOpenNISkeleton.h>
// Always include last!
#include <sawOpenNI/sawOpenNIExport.h>


#define WAIT_AND_UPDATE_ALL         1
#define WAIT_ANY_UPDATE_ONE         2
#define WAIT_NONE_UPDATE_ALL        3

#define CNI_USR_NEW         0
#define CNI_USR_LOST        1
#define CNI_USR_POSE        2
#define CNI_USR_CAL_START   3
#define CNI_USR_CAL_END     4
#define CNI_USR_SUCCESS     5
#define CNI_USR_FAIL        6
#define CNI_USR_WAIT        7
#define CNI_USR_IDLE        -1

#define CNI_HEA     1
#define CNI_NEK     2     //XN_SKEL_NECK			= 2,
#define CNI_TOR     3     //XN_SKEL_TORSO			= 3,
#define CNI_WAI     4     //XN_SKEL_WAIST			= 4,

#define CNI_LCO     5     //XN_SKEL_LEFT_COLLAR		= 5,
#define CNI_LSH     6     //XN_SKEL_LEFT_SHOULDER	= 6,
#define CNI_LEL     7     //XN_SKEL_LEFT_ELBOW		= 7,
#define CNI_LWR     8     //XN_SKEL_LEFT_WRIST		= 8,
#define CNI_LHN     9     //XN_SKEL_LEFT_HAND		= 9,
#define CNI_LFI     10    //XN_SKEL_LEFT_FINGERTIP	=10,

#define CNI_RCO     11    //XN_SKEL_RIGHT_COLLAR	=11,
#define CNI_RSH     12    //XN_SKEL_RIGHT_SHOULDER	=12,
#define CNI_REL     13    //XN_SKEL_RIGHT_ELBOW		=13,
#define CNI_RWR     14    //XN_SKEL_RIGHT_WRIST		=14,
#define CNI_RHN     15    //XN_SKEL_RIGHT_HAND		=15,
#define CNI_RFI     16    //XN_SKEL_RIGHT_FINGERTIP	=16,

#define CNI_LHP     17    //XN_SKEL_LEFT_HIP		=17,
#define CNI_LKN     18    //XN_SKEL_LEFT_KNEE		=18,
#define CNI_LAN     19    //XN_SKEL_LEFT_ANKLE		=19,
#define CNI_LFT     20    //XN_SKEL_LEFT_FOOT		=20,

#define CNI_RHP     21    //XN_SKEL_RIGHT_HIP		=21,
#define CNI_RKN     22    //XN_SKEL_RIGHT_KNEE		=22,
#define CNI_RAN     23    //XN_SKEL_RIGHT_ANKLE		=23,
#define CNI_RFT     24     //XN_SKEL_RIGHT_FOOT		=24	


/*!
  \todo move ctor code to Configure method
  \todo move all use of OpenNI symbols to .cpp file, i.e. do not include Xn files in osaOpenNI.h
  \todo move openNISkeleton class as sub class of OpenNI
  \todo add CMN_LOG_
  \todo add std cisst headers
  \todo cleanup CMakeLists, no auto find
  \todo follow cisst naming convention
*/

class osaOpenNIData;

class CISST_EXPORT osaOpenNI {

    friend class osaOpenNISkeleton;

 public:

    enum Errno {ESUCCESS, EFAILURE};

 private:

    //! Private OpenNI Data Structure
    osaOpenNIData* Data;

    //! Identifier for the openNI Object
    std::string name;

    std::vector<osaOpenNISkeleton*> skeletons;

    int users;
    bool usingPrecapturedCalibration;

    char* ProjectivePointsBuffer;
    char* WorldPointsBuffer;
    unsigned int PointsBufferSize;

 public:

    //! Default Constructor
    osaOpenNI(int numUsers = 0);

    //! Constructor for Predefined Skeleton Calibration FIles
    osaOpenNI(int numUsers, char usrPath);

    //! Default DeConstructor
    ~osaOpenNI();

    //! Clean Up and Exit
    void CleanupExit();

    //! Configure
    /**
       Creates all nodes, populates callback lists and establishes contexts for depth, rgbImage
       and users.
    */
    void Configure(const std::string & devname = "");

    //! Update All
    /**
       Calls the wait and update all method of the XN wrapper.
       This needs to be called each iteration.
    */
    void Update(int type);

    //! Init skeletons
    void InitSkeletons();

    //! Get range data
    /**
       Query the depth generator to obtain a depth image and convert the image to
       a points cloud. If the depth image is MxN, then the point cloud is a matrix
       of size 3x(MxN) where each column are the X-Y-Z coordinate of a point. This
       method is non-const due to updating the context.
       \return A 3x(MxN) point cloud.
    */
    osaOpenNI::Errno GetRangeData(vctDynamicMatrix<double> & rangedata,
                                  const std::vector< vctFixedSizeVector<unsigned short, 2> > & pixels);
    osaOpenNI::Errno GetRangeData(vctDynamicMatrixRef<vctFloat3> rangedata);

    //! Get Raw Depth Image
    /**
       Query the depth generator to obtain a depth image. Resulting image as 8-bit depth.
       The value at each pixel represents the depth of the picture element. This method
       is non-const due to updating the context.
    */
    osaOpenNI::Errno GetDepthImageRaw(vctDynamicMatrix<double> & depthimage);
    osaOpenNI::Errno GetDepthImageRaw(vctDynamicMatrixRef<unsigned short> depthimage);

    //! Get Depth Image
    /**
       Query the depth generator to obtain a depth image. Resulting image as 11-bit depth.
       The value at each pixel represents the depth of the picture element. This method
       is non-const due to updating the context.
    */
    void GetDepthImage(vctDynamicMatrix<double> & placeHolder);

    //! Get (interlaced) RGB image
    /**

     */
    osaOpenNI::Errno GetRGBImage(vctDynamicMatrix<unsigned char> & RGBinterlaced);
    osaOpenNI::Errno GetRGBImage(vctDynamicMatrixRef<unsigned char> RGBinterlaced);

    //! Get (planar) RGB image
    /**
     */
    osaOpenNI::Errno GetRGBPlanarImage(vctDynamicNArray<unsigned char,3> & RGBplanar);


    //! Get Current User Skeletons
    /**
     */
    std::vector<osaOpenNISkeleton*> & UpdateAndGetUserSkeletons(void);

    //! Get Current User Skeletons
    /**
     */
    void UpdateUserSkeletons(void);

    //! Get Current User Skeletons
    /**
     */
    std::vector<osaOpenNISkeleton*> & GetUserSkeletons(void);

    // Convert 3D to projective
    osaOpenNI::Errno
        Convert3DToProjectiveMask(const vctDynamicMatrix<double> & rangedata,
                                  vctDynamicMatrix<bool> & mask);
    void Convert3DToProjective(const vctFloat3& point3d, vctFloat3& point2d);

};

#endif
