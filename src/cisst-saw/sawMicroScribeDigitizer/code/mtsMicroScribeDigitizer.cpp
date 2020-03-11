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

#include <cisstOSAbstraction/osaGetTime.h>
#include <cisstMultiTask/mtsInterfaceProvided.h>
#include <sawMicroScribeDigitizer/mtsMicroScribeDigitizer.h>

CMN_IMPLEMENT_SERVICES(mtsMicroScribeDigitizerInfo);
CMN_IMPLEMENT_SERVICES(mtsMicroScribeDigitizer);

//---------------------------------------------------------
//  Names of Standardized Interface, Commands, and Events
//
// Name of standardized interface
const std::string mtsMicroScribeDigitizer::DigitizerInterfaceName = "Digitizer";
// Name of standardized commands
const std::string mtsMicroScribeDigitizer::DigitizerCommandNames::GetDeviceStatus = "GetDeviceStatus";
const std::string mtsMicroScribeDigitizer::DigitizerCommandNames::GetDigitizerInfo = "GetDigitizerInfo";
const std::string mtsMicroScribeDigitizer::DigitizerCommandNames::GetTipPosition = "GetTipPosition";
const std::string mtsMicroScribeDigitizer::DigitizerCommandNames::GetTipOrientation = "GetTipOrientation";
const std::string mtsMicroScribeDigitizer::DigitizerCommandNames::GetTipOrientationUnitVector = "GetTipOrientationUnitVector";
const std::string mtsMicroScribeDigitizer::DigitizerCommandNames::GetButtonState = "GetButtonState";
const std::string mtsMicroScribeDigitizer::DigitizerCommandNames::GetJointReadings = "GetJointReadings";
// Name of standardized events 
const std::string mtsMicroScribeDigitizer::DigitizerEventNames::EventButton1Up = "EventButton1Up";
const std::string mtsMicroScribeDigitizer::DigitizerEventNames::EventButton1Down = "EventButton1Down";
const std::string mtsMicroScribeDigitizer::DigitizerEventNames::EventButton2Up = "EventButton2Up";
const std::string mtsMicroScribeDigitizer::DigitizerEventNames::EventButton2Down = "EventButton2Down";
const std::string mtsMicroScribeDigitizer::DigitizerEventNames::EventDigitizerConnected = "EventDigitizerConnected";
const std::string mtsMicroScribeDigitizer::DigitizerEventNames::EventDigitizerDisconnected = "EventDigitizerDisconnected";

//---------------------------------------------------------
//  Error Handlers
//
mtsMicroScribeDigitizerInfo::mtsMicroScribeDigitizerInfo(const mtsMicroScribeDigitizerInfo & other)
    : mtsGenericObject(other)
{
    this->ProductName = other.ProductName;
    this->ModelName = other.ModelName;
    this->SerialNumber = other.SerialNumber;
    this->DriverVersion = other.DriverVersion;
    this->FirmwareVersion = other.FirmwareVersion;
	this->NumDoF = other.NumDoF;
}

void mtsMicroScribeDigitizerInfo::ToStream(std::ostream & outputStream) const
{
    mtsGenericObject::ToStream(outputStream);
    outputStream << ", Product Name: " << this->ProductName
                 << ", Model Name: " << this->ModelName
                 << ", Serial Number: " << this->SerialNumber
                 << ", Driver Version: " << this->DriverVersion
                 << ", Firmware Version: " << this->FirmwareVersion
				 << ", Num of DoF: " << this->NumDoF;
}

void mtsMicroScribeDigitizerInfo::SerializeRaw(std::ostream & outputStream) const
{
    mtsGenericObject::SerializeRaw(outputStream);
    cmnSerializeRaw(outputStream, this->ProductName);
    cmnSerializeRaw(outputStream, this->ModelName);
    cmnSerializeRaw(outputStream, this->SerialNumber);
    cmnSerializeRaw(outputStream, this->DriverVersion);
    cmnSerializeRaw(outputStream, this->FirmwareVersion);
	cmnSerializeRaw(outputStream, this->NumDoF);
}

void mtsMicroScribeDigitizerInfo::DeSerializeRaw(std::istream & inputStream)
{
    mtsGenericObject::DeSerializeRaw(inputStream);
    cmnDeSerializeRaw(inputStream, this->ProductName);
    cmnDeSerializeRaw(inputStream, this->ModelName);
    cmnDeSerializeRaw(inputStream, this->SerialNumber);
    cmnDeSerializeRaw(inputStream, this->DriverVersion);
    cmnDeSerializeRaw(inputStream, this->FirmwareVersion);
	cmnDeSerializeRaw(inputStream, this->NumDoF);
}

//---------------------------------------------------------
//  Error Handlers
//
hci_result ErrorHandler_NoHCI(struct hci_rec * hci, hci_result condition)
{
	return NO_HCI;
}

hci_result ErrorHandler_BadPort(struct hci_rec * hci, hci_result condition)
{
	return BAD_PORT_NUM;
}

hci_result ErrorHandler_CantOpen(struct hci_rec * hci, hci_result condition)
{
    return CANT_BEGIN;
}

hci_result ErrorHandler_CantBegin(struct hci_rec * hci, hci_result condition)
{
    return CANT_BEGIN;
}

//---------------------------------------------------------
//  Component mtsMicroScribeDigitizer
//
mtsMicroScribeDigitizer::mtsMicroScribeDigitizer(void)
    : mtsTaskPeriodic("NONAME", 10 * cmn_ms, false, 1000)
{
    // NOP.  This default constructor is used by unit-tests
}

mtsMicroScribeDigitizer::mtsMicroScribeDigitizer(const std::string & taskName, const double period)
    : mtsTaskPeriodic(taskName, period, false, 5000),
      DeviceConnected(false)
{
    CheckPeriod(period);

    InitComponent();
}

mtsMicroScribeDigitizer::mtsMicroScribeDigitizer(const mtsTaskPeriodicConstructorArg &arg)
    : mtsTaskPeriodic(arg),
      DeviceConnected(false)
{
    CheckPeriod(arg.Period);

    InitComponent();
}

mtsMicroScribeDigitizer::~mtsMicroScribeDigitizer()
{
    Cleanup();
}

void mtsMicroScribeDigitizer::InitComponent(void)
{
    mtsInterfaceProvided * provided = AddInterfaceProvided(DigitizerInterfaceName);
    if (provided) {
		StateTable.AddData(DeviceStatus, "DeviceStatus");
        StateTable.AddData(TipPosition, "TipPosition");
        StateTable.AddData(TipOrientation, "TipOrientation");
        StateTable.AddData(TipOrientationUnitVector, "TipOrientationUnitVector");
        StateTable.AddData(ButtonState, "ButtonState");
        StateTable.AddData(JointReadings, "JointReadings");
		provided->AddCommandReadState(StateTable, DeviceStatus, DigitizerCommandNames::GetDeviceStatus);
        provided->AddCommandReadState(StateTable, TipPosition, DigitizerCommandNames::GetTipPosition);
        provided->AddCommandReadState(StateTable, TipOrientation, DigitizerCommandNames::GetTipOrientation);
        provided->AddCommandReadState(StateTable, TipOrientationUnitVector, DigitizerCommandNames::GetTipOrientationUnitVector);
        provided->AddCommandReadState(StateTable, ButtonState, DigitizerCommandNames::GetButtonState);
        provided->AddCommandReadState(StateTable, JointReadings, DigitizerCommandNames::GetJointReadings);
		provided->AddCommandRead(&mtsMicroScribeDigitizer::GetDigitizerInfo, this, DigitizerCommandNames::GetDigitizerInfo);

        provided->AddEventVoid(EventButton1Up,    DigitizerEventNames::EventButton1Up);
        provided->AddEventVoid(EventButton1Down,  DigitizerEventNames::EventButton1Down);
        provided->AddEventVoid(EventButton2Up,    DigitizerEventNames::EventButton2Up);
        provided->AddEventVoid(EventButton2Down,  DigitizerEventNames::EventButton2Down);
        provided->AddEventVoid(EventDigitizerConnected, DigitizerEventNames::EventDigitizerConnected);
        provided->AddEventVoid(EventDigitizerDisconnected, DigitizerEventNames::EventDigitizerDisconnected);
    }

    // Device initialization
    if (ARM_SUCCESS != ArmStart(NULL)) {
		// Retry after calling ArmEnd()
		ArmEnd();
		if (ARM_SUCCESS != ArmStart(NULL)) {
			cmnThrow("mtsMicroScribeDigitizer: Startup: Failed to initialize MicroScribe Digitizer");
		}
    }

    // Custom error handler registration
    if (ARM_SUCCESS != ArmSetErrorHandlerFunction(NO_HCI_HANDLER, ErrorHandler_NoHCI)) {
        cmnThrow("mtsMicroScribeDigitizer: Startup: Failed to register error handler: NO_HCI_HANDLER");
    }
    if (ARM_SUCCESS != ArmSetErrorHandlerFunction(BAD_PORT_HANDLER, ErrorHandler_BadPort)) {
        cmnThrow("mtsMicroScribeDigitizer: Startup: Failed to register error handler: BAD_PORT_HANDLER");
    }
    if (ARM_SUCCESS != ArmSetErrorHandlerFunction(CANT_OPEN_HANDLER, ErrorHandler_CantOpen)) {
        cmnThrow("mtsMicroScribeDigitizer: Startup: Failed to register error handler: CANT_OPEN_HANDLER");
    }
    if (ARM_SUCCESS != ArmSetErrorHandlerFunction(CANT_BEGIN_HANDLER, ErrorHandler_CantBegin)) {
        cmnThrow("mtsMicroScribeDigitizer: Startup: Failed to register error handler: CANT_BEGIN_HANDLER");
    }
}

double mtsMicroScribeDigitizer::CheckPeriod(const double periodInSec)
{
    if (periodInSec < ((double) MINIMUM_PERIOD_UPDATE) * cmn_ms) {
        CMN_LOG_CLASS_INIT_WARNING << "mtsMicroScribeDigitizer: smaller period than threshold (" 
            << MINIMUM_PERIOD_UPDATE << " msec) is specified - Adjusted update period to " 
            << MINIMUM_PERIOD_UPDATE << " msec." << std::endl;
		return MINIMUM_PERIOD_UPDATE;
    }

    if (periodInSec > ((double) MAXIMUM_PERIOD_UPDATE) * cmn_ms) {
        CMN_LOG_CLASS_INIT_WARNING << "mtsMicroScribeDigitizer: larger period than threshold (" 
            << MAXIMUM_PERIOD_UPDATE << " msec) is specified - Adjusted update period to " 
            << MAXIMUM_PERIOD_UPDATE << " msec." << std::endl;
		return MAXIMUM_PERIOD_UPDATE;
    }

	return periodInSec;
}

void mtsMicroScribeDigitizer::Configure(const std::string & filename)
{
    // Setup error handlers
#define SETUP_ERROR_HANDLER(_errorCode, _handlerName)\
    if (ARM_SUCCESS != ArmSetErrorHandlerFunction(_errorCode, ErrorHandler_##_handlerName)) {\
        CMN_LOG_CLASS_INIT_ERROR << "Startup: Failed to set error handler: "#_errorCode  << std::endl;\
        cmnThrow("Startup: Failed to set error handler: "#_errorCode);\
    }
    SETUP_ERROR_HANDLER(NO_HCI_HANDLER,     NoHCI);
    SETUP_ERROR_HANDLER(BAD_PORT_HANDLER,   BadPort);
    SETUP_ERROR_HANDLER(CANT_OPEN_HANDLER,  CantOpen);
    SETUP_ERROR_HANDLER(CANT_BEGIN_HANDLER, CantBegin);
    // MJ: TIMED_OUT_HANDLER and BAD_PACKET_HANDLER are not overridden because the SDK explicitly 
	// recommends not to do that as follows (see BAD_PACKET_handler and TIMED_OUT_handler):
	//
	// "Although this function is still exported for backward compatibility issue, user 
	// shouldn't have any reason to provide any custom TIMED_OUT_handler because the default 
	// ArmDll32's TIMED_OUT_handler should be enough. Any incorrect behavior in such custom 
	// error handler function could affect ArmDll32's performance."
#undef SETUP_ERROR_HANDLER

    // Connect to hardware
    if (ARM_SUCCESS != ArmConnect(0, 0)) {
        CMN_LOG_CLASS_INIT_ERROR << "Startup: Unable to connect to ArmDll32" << std::endl;
        ArmEnd();
        cmnThrow("Startup: Unable to connect to ArmDll32");
    } else {
        OnDeviceConnection();
    }

    // Get device product information
    const size_t len = 256;
    char buf[len];
    // Product name
    if (ARM_SUCCESS == ArmGetProductName(buf, len)) {
        DigitizerInfo.ProductName = buf;
    } else {
        DigitizerInfo.ProductName = "Failed to fetch";
    }
    // Model name
    if (ARM_SUCCESS == ArmGetModelName(buf, len)) {
        DigitizerInfo.ModelName = buf;
    } else {
        DigitizerInfo.ModelName = "Failed to fetch";
    }
    // Serial number
    if (ARM_SUCCESS == ArmGetSerialNumber(buf, len)) {
        DigitizerInfo.SerialNumber = buf;
    } else {
        DigitizerInfo.SerialNumber = "Failed to fetch";
    }
    // Driver and firmware version
	char buf2[len];
    if (ARM_SUCCESS == ArmGetVersion(buf, buf2, len)) {
        DigitizerInfo.DriverVersion = buf;
        DigitizerInfo.FirmwareVersion = buf2;
    } else {
        DigitizerInfo.DriverVersion = "Failed to fetch";
        DigitizerInfo.FirmwareVersion = "Failed to fetch";
    }
	// Num of DoF
	int dof;
	if (ARM_SUCCESS == ArmGetNumDOF(&dof)) {
		// -1 if DoF cannot be determined (e.g., pre-G2 versions of the MicroScribe)
		DigitizerInfo.NumDoF = dof;
    } else {
        DigitizerInfo.NumDoF = -1;
    }

    CMN_LOG_CLASS_INIT_VERBOSE << "Startup: " << DigitizerInfo << std::endl;

    // Set refresh rate
    if (ARM_SUCCESS != ArmSetUpdateEx(ARM_FULL, (UINT) (this->Period * 1000))) { // second argument in msec
        CMN_LOG_CLASS_INIT_ERROR << "Startup: Unable to set update for ArmDll32" << std::endl;
        ArmDisconnect();
        ArmEnd();
        cmnThrow("Startup: Unable to set update for ArmDll32");
    }

	// Check device status (device can be disonncected after all getters succeeded)
	if (ARM_SUCCESS == ArmGetDeviceStatus(&DeviceStatusVendor)) {
		DeviceStatus(STATUS) = DeviceStatusVendor.status;
		DeviceStatus(BAUD) = DeviceStatusVendor.Baud;
		DeviceStatus(PORT_NUMBER) = DeviceStatusVendor.PortNumber;

		if (!(DeviceStatus(STATUS) & ARM_CONNECTED)) {
			OnDeviceDisconnection();
			return;
		}
	}
}

void mtsMicroScribeDigitizer::Startup(void)
{
}

void mtsMicroScribeDigitizer::Run(void)
{
    // timestamp of last reconnect trial in case of disconnection
    static double lastReconnectTryTime = 0;

    ProcessQueuedCommands();

    // If not connected, try reconnect at every 1 sec
    if (!DeviceConnected) {
        if (lastReconnectTryTime == 0) {
            lastReconnectTryTime = osaGetTime();
        } else if (lastReconnectTryTime + 1.0 * cmn_s < osaGetTime()) {
			CMN_LOG_CLASS_INIT_VERBOSE << "Run: Try reconnecting to digitizer..." << std::endl;
            if (ARM_SUCCESS == ArmReconnect()) {
                OnDeviceConnection();
			} else {
				CMN_LOG_CLASS_INIT_VERBOSE << "Run: Failed to reconnect to digitizer." << std::endl;
			}
        }
        return;
    }

    // read digitizer tip position
    if (ARM_NOT_CONNECTED == ArmGetTipPosition(&TipPositionVendor)) {
        // Disconnection detected
        OnDeviceDisconnection();
        return;
    }
    // read digitizer tip orientation 
    if (ARM_NOT_CONNECTED == ArmGetTipOrientation(&TipOrientationVendor)) {
        // Disconnection detected
        OnDeviceDisconnection();
        return;
    }
    // read digitizer tip orientation unit vector
    if (ARM_NOT_CONNECTED == ArmGetTipOrientationUnitVector(&TipOrientationUnitVectorVendor)) {
        // Disconnection detected
        OnDeviceDisconnection();
        return;
    }
    // read digitizer button states
    if (ARM_NOT_CONNECTED == ArmGetButtonsState(&ButtonStateVendor)) {
        // Disconnection detected
        OnDeviceDisconnection();
        return;
    }
    // read digitizer joint readings
    if (ARM_NOT_CONNECTED == ArmGetJointAngles(ARM_DEGREES, // or ARM_RADIANS
                                               JointReadingsVendor)) 
    {
        // Disconnection detected
        OnDeviceDisconnection();
        return;
    }

    // Convert vendor-defined container to cisst container
    // position
    TipPosition(X) = TipPositionVendor.x;
    TipPosition(Y) = TipPositionVendor.y;
    TipPosition(Z) = TipPositionVendor.z;
    // orientation
    TipOrientation(ROLL) = TipOrientationVendor.x;
    TipOrientation(PITCH) = TipOrientationVendor.y;
    TipOrientation(YAW) = TipOrientationVendor.z;
    // orientation unit vector
    TipOrientationUnitVector(ROLL) = TipOrientationUnitVectorVendor.x;
    TipOrientationUnitVector(PITCH) = TipOrientationUnitVectorVendor.y;
    TipOrientationUnitVector(YAW) = TipOrientationUnitVectorVendor.z;
    // button state
    static DWORD lastButtonStateVendor = 0;
    bool buttonStateChange = false;

    if (!(lastButtonStateVendor & ARM_BUTTON_1) && (ButtonStateVendor & ARM_BUTTON_1)) {
        EventButton1Down();
        ButtonState(BUTTON_1) = DOWN;
        buttonStateChange = true;
    } else if ((lastButtonStateVendor & ARM_BUTTON_1) && !(ButtonStateVendor & ARM_BUTTON_1)) {
        EventButton1Up();
        ButtonState(BUTTON_1) = UP;
        buttonStateChange = true;
    }

    if (!(lastButtonStateVendor & ARM_BUTTON_2) && (ButtonStateVendor & ARM_BUTTON_2)) {
        EventButton2Down();
        ButtonState(BUTTON_2) = DOWN;
        buttonStateChange = true;
    } else if ((lastButtonStateVendor & ARM_BUTTON_2) && !(ButtonStateVendor & ARM_BUTTON_2)) {
        EventButton2Up();
        ButtonState(BUTTON_2) = UP;
        buttonStateChange = true;
    }

    if (buttonStateChange) {
        lastButtonStateVendor = ButtonStateVendor;
    }

    // Joint readings
    for (size_t i = 0; i < NUM_DOF; ++i) {
        JointReadings(i) = JointReadingsVendor[i];
    }
}

void mtsMicroScribeDigitizer::Cleanup(void)
{
    if (DeviceConnected) {
        ArmDisconnect();
        ArmEnd();

        DeviceConnected = false;

        CMN_LOG_CLASS_INIT_VERBOSE << "Cleanup: MicroScribe Digitizer session cleaned up." << std::endl;
    }
}

void mtsMicroScribeDigitizer::GetDigitizerInfo(mtsMicroScribeDigitizerInfo & info) const
{
	info = DigitizerInfo;
}

void mtsMicroScribeDigitizer::OnDeviceConnection(void)
{
    DeviceConnected = true;

    // Notify all connected components of connection event
    EventDigitizerConnected();

    CMN_LOG_CLASS_INIT_VERBOSE << "Successfully connected to Digitizer" << std::endl;
}

void mtsMicroScribeDigitizer::OnDeviceDisconnection(void)
{
    DeviceConnected = false;

    // Notify all connected components of disconnection event
    EventDigitizerDisconnected();

    CMN_LOG_CLASS_INIT_ERROR << "Run: Digitizer disconnected" << std::endl;
}
