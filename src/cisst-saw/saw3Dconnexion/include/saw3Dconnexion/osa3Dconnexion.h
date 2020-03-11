/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */
/*

  Author(s): Simon Leonard
  Created on: Jan 11 2012

  (C) Copyright 2008 Johns Hopkins University (JHU), All Rights
  Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---
*/

#ifndef _osa3Dconnexion_h
#define _osa3Dconnexion_h

#include <saw3Dconnexion/saw3DconnexionExport.h>
#include <string>

class CISST_EXPORT osa3Dconnexion {

 public:

    enum Errno{ ESUCCESS, EFAILURE };

    struct Event{

        enum Type { UNKNOWN, MOTION, BUTTON_PRESSED, BUTTON_RELEASED };
        enum Button { BUTTON1, BUTTON2 };
        typedef long long Data[6];

        Type type;
        Button button;
        Data data;
        unsigned int timestamp;

    };

 private:

    struct Internals;
    osa3Dconnexion::Internals* internals;

    osa3Dconnexion::Errno LEDOn();
    osa3Dconnexion::Errno LEDOff();

 public:

    osa3Dconnexion();
    ~osa3Dconnexion();

    osa3Dconnexion::Errno Open( const std::string& filename = "" );
    osa3Dconnexion::Errno Close();

    osa3Dconnexion::Event WaitForEvent();

};

#endif
