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

/*!
  \file DigitizerUserComponentFLTK
  \brief Example of user component to use sawMicroScribeDigitizer component
  \ingroup sawComponents
*/

#ifndef _DigitizerUserFLTK_h
#define _DigitizerUserFLTK_h

#include "DigitizerUserComponent.h"
#include "DigitizerExampleUI.h" // FLTK GUI

class DigitizerUserComponentFLTK : public DigitizerUserComponent
{
    // FLTK GUI
    DigitizerExampleUI UI;

    // Button states
    typedef std::vector<Fl_Progress*> ButtonStateType;
    ButtonStateType ButtonStates;

    // vector of Fl_Text_Buffer
    enum {PRODUCT_NAME = 0, 
          MODEL_NAME, 
          SERIAL_NUMBER, 
          DLL_VERSION, 
          FIRM_VERSION,
          PROTOCOL, 
          CONN_STATUS};
    std::vector<Fl_Text_Buffer*> TextBuffers;

	// Update product information
	void UpdateProductInformation(void);
    // Reset UI
    void ResetUI(void);
    // Logging
    void Log(const mtsLogMessage & log);

protected:
    // additional cisst container that are not defined in the base class
    mtsFloat3 TipOrientationUnitVector;
	mtsMicroScribeDigitizer::JointReadingType JointReadings;

    // additional commands that are not defined in the base class
    mtsFunctionRead ReadTipOrientationUnitVector;
    mtsFunctionRead ReadJointReadings;

    // override event handlers of the base class
    void OnEventButton1Up(void);
    void OnEventButton1Down(void);
    void OnEventButton2Up(void);
    void OnEventButton2Down(void);
	void OnEventDigitizerConnected(void);
	void OnEventDigitizerDisconnected(void);

public:
    DigitizerUserComponentFLTK(const std::string & taskName, double period);
	~DigitizerUserComponentFLTK();

    void Configure(const std::string & CMN_UNUSED(filename) = "");
	void Startup(void);
    void Run(void);
    void Cleanup(void);

    bool UIOpened(void) const;
};
#endif  //_DigitizerUserFLTK_h
