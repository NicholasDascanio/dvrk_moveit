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

#include <sawOpenNI/osaOpenNI.h>
#include "osaOpenNIData.h"

//----------------------------------------------------------------------------/
// Callbacks
//----------------------------------------------------------------------------/

// Callback: New user was detected
void XN_CALLBACK_TYPE User_NewUser(xn::UserGenerator& generator,
                                   XnUserID nId,
                                   void* pCookie)
{
    printf("New User %d\n", nId);

    osaOpenNIData* openNIDataObject = reinterpret_cast<osaOpenNIData*>(pCookie);
    
    if(openNIDataObject->usingPrecapCalib){
        xn::SkeletonCapability skelCap = openNIDataObject->usergenerator.GetSkeletonCap();
        skelCap.LoadCalibrationDataFromFile(nId, openNIDataObject->userCalibPath);
        skelCap.StartTracking(nId); 
    }else{
        openNIDataObject->NewUserCallback(generator,nId);
        openNIDataObject->usrState = CNI_USR_NEW;
    }
}

// Callback: An existing user was lost
void XN_CALLBACK_TYPE User_LostUser(xn::UserGenerator& generator,
                                    XnUserID nId,
                                    void* pCookie)
{
    printf("Lost user %d\n", nId);

    osaOpenNIData* openNIDataObject = reinterpret_cast<osaOpenNIData*>(pCookie);
    openNIDataObject->usrState = CNI_USR_LOST;
    openNIDataObject->usrCalState = CNI_USR_IDLE;
}


// Callback: Detected a pose
void XN_CALLBACK_TYPE UserPose_PoseDetected(xn::PoseDetectionCapability& capability,
                                            const XnChar* strPose,
                                            XnUserID nId,
                                            void* pCookie)
{
    printf("Pose %s detected for user %d\n", strPose, nId);

    osaOpenNIData* openNIDataObject = reinterpret_cast<osaOpenNIData*>(pCookie);

    openNIDataObject->UserPoseDetectedCallback(capability,strPose,nId);
    openNIDataObject->usrState = CNI_USR_POSE;
}


// Callback: Started calibration
void XN_CALLBACK_TYPE UserCalibration_CalibrationStart(xn::SkeletonCapability& capability,
                                                       XnUserID nId,
                                                       void* pCookie)
{
    printf("Calibration started for user %d\n", nId);
    osaOpenNIData* openNIDataObject = reinterpret_cast<osaOpenNIData*>(pCookie);
    openNIDataObject->usrState = CNI_USR_CAL_START;
}


// Callback: Finished calibration
void XN_CALLBACK_TYPE UserCalibration_CalibrationEnd(xn::SkeletonCapability& capability,
                                                     XnUserID nId,
                                                     XnBool bSuccess,
                                                     void* pCookie)
{
    osaOpenNIData* openNIDataObject = reinterpret_cast<osaOpenNIData*>(pCookie);

    openNIDataObject->UserCalibrationEndCallback(capability,bSuccess,nId);
    openNIDataObject->usrState = CNI_USR_CAL_END;
}


/// Methods ------------------------------------------------------------------------/
void osaOpenNIData::NewUserCallback(  xn::UserGenerator& generator,
                                      XnUserID nId)
{
    if (this->needPose) {
        usergenerator.GetPoseDetectionCap().StartPoseDetection(this->strPose, nId);
    } else {
        usergenerator.GetSkeletonCap().RequestCalibration(nId, TRUE);
    }
}


void osaOpenNIData::UserPoseDetectedCallback(xn::PoseDetectionCapability& capability,
                                             const XnChar* strPose,
                                             XnUserID nId)
{
	usergenerator.GetPoseDetectionCap().StopPoseDetection(nId);
	usergenerator.GetSkeletonCap().RequestCalibration(nId, TRUE);
}


void osaOpenNIData::UserCalibrationEndCallback(xn::SkeletonCapability& capability,
                                               XnBool bSuccess,
                                               XnUserID nId)
{
	if (bSuccess) {
        // Calibration succeeded
        printf("Calibration complete, start tracking user %d\n", nId);
        usergenerator.GetSkeletonCap().StartTracking(nId);
        usrCalState = CNI_USR_SUCCESS;
    } else {
        // Calibration failed
        printf("Calibration failed for user %d\n", nId);
        usrCalState = CNI_USR_FAIL;
        if (this->needPose) {
            usergenerator.GetPoseDetectionCap().StartPoseDetection(this->strPose, nId);
            usrCalState = CNI_USR_WAIT;
        } else {
            usergenerator.GetSkeletonCap().RequestCalibration(nId, TRUE);
        }
    }
}
