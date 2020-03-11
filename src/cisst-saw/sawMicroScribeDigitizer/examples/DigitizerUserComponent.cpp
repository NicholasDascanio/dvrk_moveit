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

#include "DigitizerUserComponent.h"

#include <cisstOSAbstraction/osaGetTime.h>
#include <cisstMultiTask/mtsInterfaceRequired.h>

CMN_IMPLEMENT_SERVICES(DigitizerUserComponent);

DigitizerUserComponent::DigitizerUserComponent(const std::string & taskName, double period)
    : mtsTaskPeriodic(taskName, period, false, 5000),
	  DigitizerConnected(false)
{
    mtsInterfaceRequired * required = AddInterfaceRequired(mtsMicroScribeDigitizer::DigitizerInterfaceName);
    if (required) {
		required->AddFunction(mtsMicroScribeDigitizer::DigitizerCommandNames::GetDeviceStatus, 
                              ReadDeviceStatus);
        required->AddFunction(mtsMicroScribeDigitizer::DigitizerCommandNames::GetTipPosition, 
                              ReadTipPosition);
        required->AddFunction(mtsMicroScribeDigitizer::DigitizerCommandNames::GetTipOrientation, 
                              ReadTipOrientation);
        required->AddFunction(mtsMicroScribeDigitizer::DigitizerCommandNames::GetButtonState, 
                              ReadButtonState);
		required->AddFunction(mtsMicroScribeDigitizer::DigitizerCommandNames::GetDigitizerInfo, 
                              ReadDigitizerInfo);

        required->AddEventHandlerVoid(&DigitizerUserComponent::OnEventButton1Up, this, 
                                      mtsMicroScribeDigitizer::DigitizerEventNames::EventButton1Up);
        required->AddEventHandlerVoid(&DigitizerUserComponent::OnEventButton1Down, this, 
                                      mtsMicroScribeDigitizer::DigitizerEventNames::EventButton1Down);
        required->AddEventHandlerVoid(&DigitizerUserComponent::OnEventButton2Up, this, 
                                      mtsMicroScribeDigitizer::DigitizerEventNames::EventButton2Up);
        required->AddEventHandlerVoid(&DigitizerUserComponent::OnEventButton2Down, this, 
                                      mtsMicroScribeDigitizer::DigitizerEventNames::EventButton2Down);
        required->AddEventHandlerVoid(&DigitizerUserComponent::OnEventDigitizerConnected, this, 
                                      mtsMicroScribeDigitizer::DigitizerEventNames::EventDigitizerConnected);
        required->AddEventHandlerVoid(&DigitizerUserComponent::OnEventDigitizerDisconnected, this, 
                                      mtsMicroScribeDigitizer::DigitizerEventNames::EventDigitizerDisconnected);
    }
}

void DigitizerUserComponent::Startup(void)
{
}

void DigitizerUserComponent::Run(void)
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

    // Fetch new values
    ReadTipPosition(TipPosition);
    ReadTipOrientation(TipOrientation);
    ReadButtonState(ButtonState);

    if (ConsoleOutput) {
        std::cout << TipPosition(mtsMicroScribeDigitizer::X) << ", "
                  << TipPosition(mtsMicroScribeDigitizer::Y) << ", "
                  << TipPosition(mtsMicroScribeDigitizer::Z) << " |\t "
                  << TipOrientation(mtsMicroScribeDigitizer::ROLL) << ", "
                  << TipOrientation(mtsMicroScribeDigitizer::PITCH) << ", "
                  << TipOrientation(mtsMicroScribeDigitizer::YAW) << " |\t "
                  << ButtonState(mtsMicroScribeDigitizer::BUTTON_1) << ":"
                  << ButtonState(mtsMicroScribeDigitizer::BUTTON_2) << std::endl;
    }
}

void DigitizerUserComponent::Cleanup(void)
{
}

void DigitizerUserComponent::EnableConsoleOutput(bool enable)
{
    ConsoleOutput = enable;
}

//---------------------------------------------------------
//  Event Handlers
//
void DigitizerUserComponent::OnEventButton1Up(void)
{
    if (ConsoleOutput) {
        std::cout << "#### Button 1 UP ####" << std::endl;
    }
}

void DigitizerUserComponent::OnEventButton1Down(void)
{
    if (ConsoleOutput) {
        std::cout << "#### Button 1 DOWN ####" << std::endl;
    }
}

void DigitizerUserComponent::OnEventButton2Up(void)
{
    if (ConsoleOutput) {
        std::cout << "#### Button 2 UP ####" << std::endl;
    }
}

void DigitizerUserComponent::OnEventButton2Down(void)
{
    if (ConsoleOutput) {
        std::cout << "#### Button 2 DOWN ####" << std::endl;
    }
}

void DigitizerUserComponent::OnEventDigitizerConnected(void)
{
    DigitizerConnected = true;
    if (ConsoleOutput) {
        std::cout << "\n#### Digitizer Connected ####\n" << std::endl;
    }

	ReadDigitizerInfo(DigitizerInfo);
	std::cout << DigitizerInfo << std::endl;
}

void DigitizerUserComponent::OnEventDigitizerDisconnected(void)
{
    DigitizerConnected = false;
    if (ConsoleOutput) {
        std::cout << "\n#### Digitizer Disconnected ####\n" << std::endl;
    }
}

//---------------------------------------------------------
//  Getters
//
mtsUInt3 DigitizerUserComponent::GetDeviceStatus(void) {
	return DeviceStatus;
}

mtsFloat3 DigitizerUserComponent::GetTipPosition(void) {
    return TipPosition;
}

mtsFloat3 DigitizerUserComponent::GetTipOrientation(void) {
    return TipOrientation;
}

mtsBool2 DigitizerUserComponent::GetButtonState(void) {
    return ButtonState;
}

mtsMicroScribeDigitizerInfo DigitizerUserComponent::GetDigitizerInfo(void) {
    return DigitizerInfo;
}

void DigitizerUserComponent::GetDeviceStatus(mtsUInt3 & deviceStatus) {
    deviceStatus = DeviceStatus;
}

void DigitizerUserComponent::GetTipPosition(mtsFloat3 & position) {
    position = TipPosition;
}

void DigitizerUserComponent::GetTipOrientation(mtsFloat3 & orientation) {
    orientation = TipOrientation;
}

void DigitizerUserComponent::GetButtonState(mtsBool2 & state) {
    state = ButtonState;
}

void DigitizerUserComponent::GetDigitizerInfo(mtsMicroScribeDigitizerInfo & digitizerInfo) {
    digitizerInfo = DigitizerInfo;
}
