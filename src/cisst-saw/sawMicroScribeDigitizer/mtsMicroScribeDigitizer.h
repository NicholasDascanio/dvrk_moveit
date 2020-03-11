/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*

  Author(s):  Min Yang Jung
  Created on: 2011-10-06

  (C) Copyright 2011 Johns Hopkins University (JHU), All Rights Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---
*/

/*!
  \file
  \brief cisst component for Immersion MicroScribe Digitizers
  \ingroup cisstComponents
*/

#ifndef _mtsMicroScribeDigitizer_h
#define _mtsMicroScribeDigitizer_h

#include <cisstMultiTask/mtsTaskPeriodic.h>
#include <cisstMultiTask/mtsFunctionVoid.h>
#include <cisstMultiTask/mtsFixedSizeVectorTypes.h>

#include <windows.h>
#include <armdll32.h>

#include <sawMicroScribeDigitizer/sawMicroScribeDigitizerExport.h>

//----------------------------------------------------------
// MicroScribe Digitizer Product Information
//
class CISST_EXPORT mtsMicroScribeDigitizerInfo: public mtsGenericObject
{
    CMN_DECLARE_SERVICES(CMN_DYNAMIC_CREATION, CMN_LOG_ALLOW_DEFAULT);

public:
    std::string ProductName;
    std::string ModelName;
    std::string SerialNumber;
    std::string DriverVersion;
    std::string FirmwareVersion;
	int			NumDoF;

    /*! Default constructor */
    mtsMicroScribeDigitizerInfo() {}
    /*! Copy constructor */
    mtsMicroScribeDigitizerInfo(const mtsMicroScribeDigitizerInfo &other);
    /*! Destructor */
    ~mtsMicroScribeDigitizerInfo() {}

    void ToStream(std::ostream & outputStream) const;
    void SerializeRaw(std::ostream & outputStream) const;
    void DeSerializeRaw(std::istream & inputStream);
};

CMN_DECLARE_SERVICES_INSTANTIATION(mtsMicroScribeDigitizerInfo);

//----------------------------------------------------------
// MicroScribe Digitizer Information
//
class CISST_EXPORT mtsMicroScribeDigitizer : public mtsTaskPeriodic
{
	friend class sawMicroScribeDigitizerTest; // for unit-tests

    CMN_DECLARE_SERVICES(CMN_DYNAMIC_CREATION_ONEARG, CMN_LOG_ALLOW_DEFAULT);

public:
    enum DIGITIZER_POSITION { X = 0, Y, Z };
    enum DIGITIZER_ORIENTATION { ROLL = 0, PITCH, YAW };
    enum DIGITIZER_BUTTON { BUTTON_1 = 0, BUTTON_2 };
    enum DIGITIZER_BUTTON_STATE { DOWN = 0, UP };
	enum DIGITIZER_DEVICE_STATUS { STATUS = 0, BAUD, PORT_NUMBER };

    // Name of standardized interface
    const static std::string DigitizerInterfaceName;
    // Name of standardized commands
    class CISST_EXPORT DigitizerCommandNames {
    public:
		const static std::string GetDeviceStatus;
        const static std::string GetDigitizerInfo;
        const static std::string GetTipPosition;
        const static std::string GetTipOrientation;
        const static std::string GetTipOrientationUnitVector;
        const static std::string GetButtonState;
        const static std::string GetJointReadings;
    };
    // Name of standardized events 
    class CISST_EXPORT DigitizerEventNames {
    public:    
        const static std::string EventButton1Up;
        const static std::string EventButton1Down;
        const static std::string EventButton2Up;
        const static std::string EventButton2Down;
		const static std::string EventDigitizerConnected;
        const static std::string EventDigitizerDisconnected;
    };

	// Typedef for joint readings
	typedef mtsFixedSizeVector<angle, NUM_DOF> JointReadingType;

protected:
    // If digitizer is open
    bool DeviceConnected;

    // Vendor-specific container
	device_status DeviceStatusVendor;
    length_3D TipPositionVendor;
    angle_3D  TipOrientationVendor;
    angle_3D  TipOrientationUnitVectorVendor;
    DWORD     ButtonStateVendor;
    angle     JointReadingsVendor[NUM_DOF];
    // cisst container
	mtsUInt3  DeviceStatus;
    mtsFloat3 TipPosition;
    mtsFloat3 TipOrientation;
    mtsFloat3 TipOrientationUnitVector;
    mtsBool2  ButtonState;
    JointReadingType JointReadings; 

    // Digitizer information
    mtsMicroScribeDigitizerInfo DigitizerInfo;

    // Button state transition events
    mtsFunctionVoid EventButton1Up;
    mtsFunctionVoid EventButton1Down;
    mtsFunctionVoid EventButton2Up;
    mtsFunctionVoid EventButton2Down;
    // Digitizer connection and disconnection event
	mtsFunctionVoid EventDigitizerConnected;
    mtsFunctionVoid EventDigitizerDisconnected;

    mtsMicroScribeDigitizer(void);

	void InitComponent(void);
	double CheckPeriod(const double periodInSec);

	void OnDeviceConnection(void);
	void OnDeviceDisconnection(void);

	void GetDigitizerInfo(mtsMicroScribeDigitizerInfo & info) const;

public:
    // Constructors and destructor
    mtsMicroScribeDigitizer(const std::string & taskName, const double period);
    mtsMicroScribeDigitizer(const mtsTaskPeriodicConstructorArg &arg);
    ~mtsMicroScribeDigitizer();

    // Methods required by mtsTask
    void Configure(const std::string & filename = "");
    void Startup(void);
    void Run(void);
    void Cleanup(void);

};

CMN_DECLARE_SERVICES_INSTANTIATION(mtsMicroScribeDigitizer);

#endif  //_mtsMicroScribeDigitizer_h
