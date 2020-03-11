/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*

  Author(s): Anton Deguet
  Created on: 2009-03-27

  (C) Copyright 2009-2011 Johns Hopkins University (JHU), All Rights Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---
*/

#ifndef _mtsSartoriusSerial_h
#define _mtsSartoriusSerial_h

//includes from cisstlibraries
#include <cisstCommon/cmnPortability.h>
#include <cisstOSAbstraction/osaSerialPort.h>
#include <cisstMultiTask/mtsTaskContinuous.h>
#include <cisstMultiTask/mtsGenericObjectProxy.h>

// Always include last
#include <sawSartoriusScale/sawSartoriusScaleExport.h>

/*!
  \brief Device wrapper for Sartorius scale (model GC 2502)

  This component assumes the following settings on the scale:
  - Serial port rate 19200, 7 chars, Odd parity checking, One bit stop and hardware flow control
  - Continuous print (menu 6 1 4)
*/

class CISST_EXPORT mtsSartoriusSerial: public mtsTaskContinuous
{
    CMN_DECLARE_SERVICES(CMN_NO_DYNAMIC_CREATION, CMN_LOG_ALLOW_DEFAULT);

 protected:

    /*! Set serial port parameters to match scale's defaults */
    void SetSerialPortDefaults(void);

    /*! Setup interface and state table */
    void SetupInterface(void);

    /*! "System" Commands: */
    void SendPrintToggle(void);

 public:
    bool GetWeight(double & weightInGrams, bool & stable);
 protected:
    bool GetModel(std::string & modelName);
    bool ProcessBuffer(void);
    typedef const char * const_char_pointer;
    void UpdateStateTable(const const_char_pointer & buffer);

    /*! Placeholder for last weigth read */
    mtsDouble Weight;

    /*! Replies are limited to 16 chars, to be tested */
    enum {BUFFER_SIZE = 512};
    char BytesReadSoFar[BUFFER_SIZE];
    unsigned int NbBytesReadSoFar;
    char TempBuffer[BUFFER_SIZE];

    /*! Serial port instance */
    osaSerialPort SerialPort;

 public:

    mtsSartoriusSerial(const std::string & taskName,
                       const std::string & serialPortName);
    mtsSartoriusSerial(const std::string & taskName,
                       unsigned int serialPortNumber);

    // all four methods are pure virtual in mtsTaskContinous
    void Configure(const std::string & CMN_UNUSED(filename)) {};
    void Startup(void);
    void Run(void);
    void Cleanup(void);

};

CMN_DECLARE_SERVICES_INSTANTIATION(mtsSartoriusSerial);

#endif //_mtsSartoriusSerial_h_
