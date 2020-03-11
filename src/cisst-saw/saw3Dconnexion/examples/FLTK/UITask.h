/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*

  Author(s):  Marcin Balicki
  Created on: 2008-04-12

  (C) Copyright 2008-2011 Johns Hopkins University (JHU), All Rights
  Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---
*/

#ifndef _UITask_h
#define _UITask_h

#include <cisstMultiTask/mtsTaskPeriodic.h>
#include <cisstMultiTask/mtsVector.h>

// FLTK/fluid generated header file
#include "UI.h"

class UITask: public mtsTaskPeriodic {
    CMN_DECLARE_SERVICES(CMN_NO_DYNAMIC_CREATION, 5);
    volatile bool ExitFlag;

protected:
    UI Mouse3DGUI;
    //6 degree input from the 3d mouse.
    //it is displacement from 0 , proportional to force torque readings (but not precise
    mtsDoubleVec AnalogInput;
    mtsBoolVec DigitalInput;

    mtsFunctionRead GetAnalogInput;
    mtsFunctionRead GetDigitalInput;

public:
    UITask(const std::string & taskName,
           double period);
    ~UITask() {};

    void Configure(const std::string & filename = "");
    void Startup(void);
    void Run(void);
    void Cleanup(void) {};

    bool GetExitFlag (void);
};

CMN_DECLARE_SERVICES_INSTANTIATION(UITask);

#endif // _UITask_h
