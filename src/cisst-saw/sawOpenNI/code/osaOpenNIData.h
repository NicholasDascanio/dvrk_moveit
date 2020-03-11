/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*

  Author(s):  Kelleher Guerin, Simon Leonard
  Created on: 2011

  (C) Copyright 2011 Johns Hopkins University (JHU), All Rights
  Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---

*/

#ifndef _osaOpenNIData_h
#define _osaOpenNIData_h

#include <sawOpenNI/osaOpenNI.h>

#include <XnCppWrapper.h>
#include <XnOS.h>

class osaOpenNIData
{

public:

    //! OpenNI context
    xn::Context context;

    //! Depth image generator
    xn::DepthGenerator depthgenerator;

    //! RGB image generator
    xn::ImageGenerator rgbgenerator;

    //! User Skeleton generator
    xn::UserGenerator usergenerator;

    //! User Pose State
    XnBool needPose;

    //! User State
    int usrState;

    //! User Calibration State
    int usrCalState;

    //! Using Precaptured Calibration Data
    bool usingPrecapCalib;
    XnChar* userCalibPath;

    //! Pose Callback String
    XnChar strPose[20];

    //! New User Callback
    /**
     */
    void NewUserCallback(xn::UserGenerator& generator,
                         XnUserID nId);

    //! New User Calibration Pose Detected
    /**
     */
    void UserPoseDetectedCallback(xn::PoseDetectionCapability& capability,
                                  const XnChar* strPose,
                                  XnUserID nId);

    //! User Calibration End
    /**
       Cues that the calibration has ended, either with success or failure.
    */
    void UserCalibrationEndCallback(xn::SkeletonCapability& capability,
                                    XnBool bSuccess,
                                    XnUserID nId);
    void SetStates(){

        usrState = CNI_USR_IDLE;
        usrCalState = CNI_USR_IDLE;

    };


};

//----------------------------------------------------------------------------/
// BEGIN Callbacks
//----------------------------------------------------------------------------/

// Callback: New user was detected
void XN_CALLBACK_TYPE User_NewUser(xn::UserGenerator& generator,
                                   XnUserID nId,
                                   void* pCookie);
// Callback: An existing user was lost
void XN_CALLBACK_TYPE User_LostUser(xn::UserGenerator& generator,
                                    XnUserID nId,
                                    void* pCookie);


// Callback: Detected a pose
void XN_CALLBACK_TYPE UserPose_PoseDetected(xn::PoseDetectionCapability& capability,
                                            const XnChar* strPose,
                                            XnUserID nId,
                                            void* pCookie);

// Callback: Started calibration
void XN_CALLBACK_TYPE UserCalibration_CalibrationStart(xn::SkeletonCapability& capability,
                                                       XnUserID nId,
                                                       void* pCookie);

// Callback: Finished calibration
void XN_CALLBACK_TYPE UserCalibration_CalibrationEnd(xn::SkeletonCapability& capability,
                                                     XnUserID nId,
                                                     XnBool bSuccess,
                                                     void* pCookie);


#endif
