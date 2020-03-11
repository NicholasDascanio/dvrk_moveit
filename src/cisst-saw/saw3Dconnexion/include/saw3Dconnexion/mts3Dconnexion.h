/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*
  $Id: mts3Dconnexion.h 3243 2011-12-10 02:43:59Z adeguet1 $

  Author(s):  Marcin Balicki, Anton Deguet
  Created on: 2008-04-12

  (C) Copyright 2008-2012 Johns Hopkins University (JHU), All Rights
  Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---
*/

/*!
  \file
  \brief SAW component for 3Dconnexion space navigator mice.
  \ingroup sawComponents

  \warning On Windows, buttons need to be configured as Button1 and Button2.
  \warning On Mac, one needs the RunLoop to catch and propagate events and all
           event handlers must be created/registered in the main loop (unless
           we figure out how to run an event loop (cocoa based) on top of a
           posix thread.

  \todo Can we activate the buttons from code, i.e. not using external 3Dconnexion control panel.
  \todo Use prm type for API? At osa level, use vctTypes?
  \todo Add calibrate/bias function.
  \todo Add bypassing wizard settings, looks like overall speed setting should be at max, button numbers, ...
  \todo Test connection robustness.
  \todo Standardize values. (max is 1600 with full speed setting for both trans and rot).
  \todo Add button events.
  \todo Check update rate seems sluggish with latency.
  \todo Remove the loop timer, use the automatic one.
  \todo Use wizard to set the buttons to 1, and 2 (keystroke #s).
*/

#ifndef _mts3Dconnexion_h
#define _mts3Dconnexion_h

#include <cisstMultiTask/mtsTaskPeriodic.h>
#include <cisstMultiTask/mtsVector.h>
#include <cisstParameterTypes/prmPositionCartesianGet.h>
#include <saw3Dconnexion/saw3DconnexionExport.h>  // always include last


class mts3DconnexionData;  // class containing OS specific data


class CISST_EXPORT mts3Dconnexion: public mtsTaskPeriodic
{
    CMN_DECLARE_SERVICES(CMN_DYNAMIC_CREATION_ONEARG, CMN_LOG_ALLOW_DEFAULT);

    // platform "friendly" message handler for Cocoa events on Mac
    friend void mts3DconnexionInternalMessageHandler(mts3Dconnexion * instance, const vctDynamicVector<double> & axis, const vctDynamicVector<bool> & buttons);

 public:
    /*! Constructors */
    mts3Dconnexion(const std::string & taskName, double period) :
        mtsTaskPeriodic(taskName, period, false, 500) {}
    mts3Dconnexion(const mtsTaskPeriodicConstructorArg & arg) :
        mtsTaskPeriodic(arg) {}

    /*! Destructor */
    ~mts3Dconnexion(void) {}

    /*! Device needs to be configured on the thread running the event loop
        (main thread) on Mac. */
    void Configure(const std::string & CMN_UNUSED(configurationName) = "");
    /*! Device needs to be configured on the component thread on Windows. */
    void Startup(void);
    void Run(void);
    void Cleanup(void);

    void ReBias(void) {
        CMN_LOG_CLASS_RUN_ERROR << "ReBias not implemented" << std::endl;
    }

 protected:
    void UpdateDataTable(void);

    mtsStateTable * DataTable;  // store data in separate state table
    mtsDoubleVec Axis;
    mtsBoolVec Buttons;
    mtsBoolVec Mask;
    mtsDouble Gain;
    prmPositionCartesianGet Position;
    mtsBool IsConnected;

    mts3DconnexionData * Data;
    std::string ConfigurationName;  // this is the name used to load the configuration settings from the 3dCon application
    vct3 Translation;
    vct3 Orientation;
};

CMN_DECLARE_SERVICES_INSTANTIATION(mts3Dconnexion);

#endif  // _mts3Dconnexion_h
