/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*

  Author(s):  Min Yang Jung
  Created on: 2011-10-07

  (C) Copyright 2011 Johns Hopkins University (JHU), All Rights Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---
*/

/*!
  \file DigitizerUserComponent
  \brief Example of user component to use sawMicroScribeDigitizer component
  \ingroup sawComponents
*/

#ifndef _DigitizerUser_h
#define _DigitizerUser_h

#include <cisstMultiTask/mtsTaskPeriodic.h>
#include <cisstMultiTask/mtsFunctionVoid.h>
#include <cisstMultiTask/mtsFixedSizeVectorTypes.h>
#include <sawMicroScribeDigitizer/mtsMicroScribeDigitizer.h>

class DigitizerUserComponent : public mtsTaskPeriodic
{
    CMN_DECLARE_SERVICES(CMN_NO_DYNAMIC_CREATION, CMN_LOG_ALLOW_DEFAULT);
    
protected:
    // if output to console is needed
    bool ConsoleOutput;
	// if digitizer is connected
	bool DigitizerConnected;

    // cisst container
	mtsUInt3  DeviceStatus;
    mtsFloat3 TipPosition;
    mtsFloat3 TipOrientation;
    mtsBool2  ButtonState;

    // Digitizer information
    mtsMicroScribeDigitizerInfo DigitizerInfo;

	mtsFunctionRead ReadDeviceStatus;
    mtsFunctionRead ReadTipPosition;
    mtsFunctionRead ReadTipOrientation;
    mtsFunctionRead ReadButtonState;
    mtsFunctionRead ReadDigitizerInfo;

    virtual void OnEventButton1Up(void);
    virtual void OnEventButton1Down(void);
    virtual void OnEventButton2Up(void);
    virtual void OnEventButton2Down(void);
	virtual void OnEventDigitizerConnected(void);
	virtual void OnEventDigitizerDisconnected(void);

public:
    DigitizerUserComponent(const std::string & taskName, double period);
	~DigitizerUserComponent() {}

    // Getters
	mtsUInt3  GetDeviceStatus(void);
    mtsFloat3 GetTipPosition(void);
    mtsFloat3 GetTipOrientation(void);
    mtsBool2  GetButtonState(void);
    mtsMicroScribeDigitizerInfo GetDigitizerInfo(void);
	void GetDeviceStatus(mtsUInt3 & deviceStatus);
    void GetTipPosition(mtsFloat3 & position);
    void GetTipOrientation(mtsFloat3 & orientation);
    void GetButtonState(mtsBool2 & state);
    void GetDigitizerInfo(mtsMicroScribeDigitizerInfo & digitizerInfo);

    void EnableConsoleOutput(bool enable = true);

    virtual void Configure(const std::string & filename = "") {}
    virtual void Startup(void);
    virtual void Run(void);
    virtual void Cleanup(void);
};

CMN_DECLARE_SERVICES_INSTANTIATION(DigitizerUserComponent);

#endif  //_DigitizerUser_h
