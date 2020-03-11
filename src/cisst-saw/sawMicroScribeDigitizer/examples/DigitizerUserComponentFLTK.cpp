/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*

  Author(s):  Min Yang Jung
  Created on: 2011-10-08

  (C) Copyright 2011 Johns Hopkins University (JHU), All Rights Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---
*/

#include <cisstOSAbstraction/osaGetTime.h>
#include <cisstMultiTask/mtsManagerComponentBase.h>

#include "DigitizerUserComponentFLTK.h"

#include <FL/Fl_Text_Buffer.H>

DigitizerUserComponentFLTK::DigitizerUserComponentFLTK(const std::string & taskName, double period)
    : DigitizerUserComponent(taskName, period)
{
    // For receiving system-wide logs from MCS
    mtsInterfaceRequired * required = AddInterfaceRequired(
        mtsManagerComponentBase::InterfaceNames::InterfaceSystemLoggerRequired,
        MTS_OPTIONAL); // MJ: in case older GCM is used
    if (required) {
        required->AddEventHandlerWrite(&DigitizerUserComponentFLTK::Log, this, 
                                       mtsManagerComponentBase::EventNames::PrintLog);
    }

    required = GetInterfaceRequired(mtsMicroScribeDigitizer::DigitizerInterfaceName);
    CMN_ASSERT(required); // base class should have this interface

    required->AddFunction(mtsMicroScribeDigitizer::DigitizerCommandNames::GetTipOrientationUnitVector, 
                          ReadTipOrientationUnitVector);
	required->AddFunction(mtsMicroScribeDigitizer::DigitizerCommandNames::GetJointReadings, 
                          ReadJointReadings);
}

DigitizerUserComponentFLTK::~DigitizerUserComponentFLTK()
{
    for (size_t i = 0; i < TextBuffers.size(); ++i)
        delete TextBuffers[i];
    delete UI.Log->buffer();
}

bool DigitizerUserComponentFLTK::UIOpened(void) const
{
    return UI.Opened;
}

void DigitizerUserComponentFLTK::Configure(const std::string & CMN_UNUSED(filename))
{
    // Link UI and UI task, set property
    UI.Button1->maximum(1.0);
    UI.Button2->maximum(1.0);

    Fl_Text_Buffer * buf;
#define CREATE_BUFFER( _ui )\
    buf = new Fl_Text_Buffer();\
    UI._ui->buffer(buf);\
    TextBuffers.push_back(buf);\

    CREATE_BUFFER(ProductName);
    CREATE_BUFFER(ModelName);
    CREATE_BUFFER(SerialNumber);
    CREATE_BUFFER(DLLVersion);
    CREATE_BUFFER(FirmwareVersion);
    CREATE_BUFFER(Protocol);
    CREATE_BUFFER(ConnectionStatus);
#undef CREATE_BUFFER

    // Logger
    UI.Log->textsize(13);
    buf = new Fl_Text_Buffer();
    UI.Log->buffer(buf);
    buf->text("");

    // Clear up
    ResetUI();

    // make the UI visible
    UI.show(0, NULL);
    UI.Opened = true;

	EnableConsoleOutput(false);
}

void DigitizerUserComponentFLTK::Startup(void)
{
}


void DigitizerUserComponentFLTK::Run(void)
{
    ProcessQueuedCommands();
	ProcessQueuedEvents();

	if (!DigitizerConnected) {
		// Check if Digitizer is back online at every 0.5 second
		static double lastCheckTime = 0;
		if (osaGetTime() - lastCheckTime > 0.5) {
			lastCheckTime = osaGetTime();

			// Update device status
			ReadDeviceStatus(DeviceStatus);
			if (DeviceStatus(mtsMicroScribeDigitizer::STATUS) & ARM_CONNECTED) {
				OnEventDigitizerConnected();
				return;
			} else {
				CMN_LOG_CLASS_INIT_ERROR << "!!! DIGITIZER DISCONNECTED !!!" << std::endl;
			}
		}
		return;
    }

	// Get new values
	// What the base class fetches:
	// - tip position
	// - tip orientation
	// - product information
	// - button states
	DigitizerUserComponent::Run();
	// What this derived class fetches:
	// - joint values
	// - tip orientation unit vector
	ReadJointReadings(JointReadings);
	ReadTipOrientationUnitVector(TipOrientationUnitVector);

	// Update UI
	if (NUM_DOF == 6) { // MicroScribe MLX has 6 DoF
		UI.J1->value(JointReadings(0));
		UI.J2->value(JointReadings(1));
		UI.J3->value(JointReadings(2));
		UI.J4->value(JointReadings(3));
		UI.J5->value(JointReadings(4));
		UI.J6->value(JointReadings(5));
	}

	UI.X->value(TipPosition(mtsMicroScribeDigitizer::X));
	UI.Y->value(TipPosition(mtsMicroScribeDigitizer::Y));
	UI.Z->value(TipPosition(mtsMicroScribeDigitizer::Z));
	
	UI.Roll->value(TipOrientation(mtsMicroScribeDigitizer::ROLL));
	UI.Pitch->value(TipOrientation(mtsMicroScribeDigitizer::PITCH));
	UI.Yaw->value(TipOrientation(mtsMicroScribeDigitizer::YAW));

	UI.Ux->value(TipOrientationUnitVector(mtsMicroScribeDigitizer::X));
	UI.Uy->value(TipOrientationUnitVector(mtsMicroScribeDigitizer::Y));
	UI.Uz->value(TipOrientationUnitVector(mtsMicroScribeDigitizer::Z));
}

void DigitizerUserComponentFLTK::Cleanup(void)
{
}

void DigitizerUserComponentFLTK::ResetUI(void)
{
#define RESET_TEXT_UI( _id ) TextBuffers[_id]->text("");
    RESET_TEXT_UI(PRODUCT_NAME);
    RESET_TEXT_UI(MODEL_NAME);
    RESET_TEXT_UI(SERIAL_NUMBER);
    RESET_TEXT_UI(DLL_VERSION);
    RESET_TEXT_UI(FIRM_VERSION);
    RESET_TEXT_UI(PROTOCOL);
#undef RESET_TEXT_UI
	TextBuffers[CONN_STATUS]->text("Disconnected");

    UI.J1->value(0);
    UI.J2->value(0);
    UI.J3->value(0);
    UI.J4->value(0);
    UI.J5->value(0);
    UI.J6->value(0);
    UI.X->value(0);
    UI.Y->value(0);
    UI.Z->value(0);
    UI.Roll->value(0);
    UI.Pitch->value(0);
    UI.Yaw->value(0);
    UI.Ux->value(0);
    UI.Uy->value(0);
    UI.Uz->value(0);

    UI.Button1->value(0.0);
    UI.Button2->value(0.0);
}

void DigitizerUserComponentFLTK::Log(const mtsLogMessage & log)
{
    std::string s(log.Message, log.Length);
   
    UI.Log->move_down();
    UI.Log->insert(s.c_str());
    UI.Log->show_insert_position();
}

void DigitizerUserComponentFLTK::UpdateProductInformation(void)
{
	// Get digitizer information
	ReadDigitizerInfo(DigitizerInfo);

#define SHOW_DIGITIZER_INFO( _id, _var ) TextBuffers[_id]->text(DigitizerInfo._var.c_str());
	SHOW_DIGITIZER_INFO(PRODUCT_NAME, ProductName);
	SHOW_DIGITIZER_INFO(MODEL_NAME, ModelName);
	SHOW_DIGITIZER_INFO(SERIAL_NUMBER, SerialNumber);
	SHOW_DIGITIZER_INFO(DLL_VERSION, DriverVersion);
	SHOW_DIGITIZER_INFO(FIRM_VERSION, FirmwareVersion);
#undef SHOW_DIGITIZER_INFO

	UI.DoF->value(DigitizerInfo.NumDoF);

	if (DeviceStatus(mtsMicroScribeDigitizer::STATUS) & ARM_USING_SERIAL_PORT) {
		std::stringstream ss;
		ss << "COM: " << DeviceStatus(mtsMicroScribeDigitizer::PORT_NUMBER) << ", "
		   << "Baud: " << DeviceStatus(mtsMicroScribeDigitizer::BAUD);
		TextBuffers[PROTOCOL]->text(ss.str().c_str());
	} else if (DeviceStatus(mtsMicroScribeDigitizer::STATUS) & ARM_USING_USB_PORT) {
		TextBuffers[PROTOCOL]->text("USB");
	}
}

//---------------------------------------------------------
//  Event Handlers
//
void DigitizerUserComponentFLTK::OnEventButton1Up(void)
{
	UI.Button1->value(0.0);

	CMN_LOG_CLASS_RUN_VERBOSE << "Event: Button 1 Up" << std::endl;
}

void DigitizerUserComponentFLTK::OnEventButton1Down(void)
{
	UI.Button1->value(1.0);

    CMN_LOG_CLASS_RUN_VERBOSE << "Event: Button 1 Down" << std::endl;
}

void DigitizerUserComponentFLTK::OnEventButton2Up(void)
{
	UI.Button2->value(0.0);

	CMN_LOG_CLASS_RUN_VERBOSE << "Event: Button 2 Up" << std::endl;
}

void DigitizerUserComponentFLTK::OnEventButton2Down(void)
{
	UI.Button2->value(1.0);

    CMN_LOG_CLASS_RUN_VERBOSE << "Event: Button 2 Down" << std::endl;
}

void DigitizerUserComponentFLTK::OnEventDigitizerConnected(void)
{
    DigitizerConnected = true;

	TextBuffers[CONN_STATUS]->text("Connected");

	ReadDeviceStatus(DeviceStatus);
	UpdateProductInformation();
    
	CMN_LOG_CLASS_INIT_VERBOSE << "Event: Digitizer connected" << std::endl;
}

void DigitizerUserComponentFLTK::OnEventDigitizerDisconnected(void)
{
    DigitizerConnected = false;

	TextBuffers[CONN_STATUS]->text("Disconnected");
    
	CMN_LOG_CLASS_INIT_ERROR << "Event: Digitizer disconnected" << std::endl;
}
