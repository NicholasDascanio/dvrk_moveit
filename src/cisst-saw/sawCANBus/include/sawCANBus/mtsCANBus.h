/*

  Author(s): Simon Leonard
  Created on: Dec 02 2009

  (C) Copyright 2009 Johns Hopkins University (JHU), All Rights
  Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---
*/


#ifndef _mtsCANBus_h
#define _mtsCANBus_h

#include <cisstMultiTask/mtsComponent.h>
#include <cisstMultiTask/mtsInterfaceProvided.h>

#include <sawCANBus/osaCANBus.h>
#include <sawCANBus/mtsCANBusFrame.h>
#include <sawCANBus/sawCANBusExport.h>

//! MTS interfaces for a sawCANBus device
/**
   This class implements 2 MTS interfaces that can be attached to any sawCANBus
   device. The first interface is for IO (read/write) and the second interface
   (CTL) is for controlling the device. The IO interface provides two methods
   for reading/writing to the CAN device. The CTL interface provides Open/Close
   methods. Both methods use the mtsCANFrame type. This class is not intended to
   be used by itself but to be "attached" to a sawCANBus device (i.e. multiple 
   inheritance).
*/
class CISST_EXPORT mtsCANBus : public mtsComponent{
  
  CMN_DECLARE_SERVICES( CMN_NO_DYNAMIC_CREATION, CMN_LOG_LOD_RUN_ERROR );

 private:

  //! The sawCANBus device
  /**
     This pointer must be set to a valid sawCANBus object. Once configured, the
     device can be used through MTS.
  */
  osaCANBus* can;

  //! Open the CAN device
  void mtsOpen();

  //! Close the CAN device
  void mtsClose();

  //! Read a CAN frame from the device
  void mtsRead( mtsCANBusFrame &frame ) const;

  //! Write a CAN frame to the device
  void mtsWrite( const mtsCANBusFrame &frame );

 protected:

  //! The input/output interface
  mtsInterfaceProvided* io;

  //! The control interface
  mtsInterfaceProvided* ctl;

 public:
  
  // Main constructor
  /**
     Creates the MTS interfaces for the CAN device and add read/write commands.
     \param componentname The MTS name of the component
     \param can The CAN device to attach to the MTS interfaces
  */
  mtsCANBus( const std::string& componentname, osaCANBus* can );

};

CMN_DECLARE_SERVICES_INSTANTIATION( mtsCANBus )

#endif
