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


#ifndef _osaSocketCAN_h
#define _osaSocketCAN_h

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include <linux/can.h>
#include <linux/can/raw.h>
#include <linux/can/error.h>
#include <linux/if.h>

#include <sawCANBus/osaCANBus.h>       // the CAN base class
#include <sawCANBus/sawCANBusExport.h>

//! A Real Time Socket CAN device
/**
   Most harware in Linux use the /dev interface. Typically, your device (usb,
   serial port, ...) will be represented by a file in /dev. CAN hardware is no
   exception except for the socket CAN module. SocketCAN implements a CAN stack
   and extend the BSD sockets just like you have a ethernet stack and sockets. 
   RT SocketCAN is the "real time" version for the Xenomai framework.
   To use osaSocketCAN, you must have a Xenomai patched Linux kernel with 
   The RT CAN module enabled
*/

class CISST_EXPORT osaSocketCAN : public osaCANBus {

 private:
  
  //! The name of the CAN device (rtcan0, rtcan1, ...)
  std::string devicename;
  
  //! The file descriptor of the socket
  int canfd;

  //! CAN filters
  /**
     The default maximum number of CAN filter in Xenomai is 16. Howerver, this
     limit can be increased when configuring the kernel.
   */
  static const size_t MAX_NUM_FILTERS = 32;
  struct can_filter filters[MAX_NUM_FILTERS];
  size_t filterscnt;

public:

  //! Constructor
  /**
     Initialize the device name and the rate of the CAN device
     \param devicename The name of the device (rtcan0, rtcan1, ...)
     \param rate The CAN rate (RATE_150, RATE_300, RATE_1000)
  */
  osaSocketCAN( const std::string& devicename, 
		  osaCANBus::Rate rate,
		  osaCANBus::Loopback loopback = osaCANBus::LOOPBACK_OFF );
  
  //! Destructor
  ~osaSocketCAN();

  //! Open and configure the CAN socket
  /**
     Creates and configure a RT CAN socket.
  */
  osaCANBus::Errno Open();

  //! Close the CAN socket
  osaCANBus::Errno Close();

  //! Send a CAN frame on the bus
  /**
     Call this method to send a CAN frame on the bus.
     \param frame[in] The CAN frame to send on the bus
     \param flags Block the device until the operation is completed. This
                  parameter is irrelevant for osaSocketCAN.
  */
  osaCANBus::Errno Send( const osaCANBusFrame& frame, 
			osaCANBus::Flags flags = osaCANBus::MSG_NOFLAG );

  //! Receive a CAN frame
  /**
     Call this method to receive a CAN frame.
     \param frame[out] The CAN frame received from the bus
     \param flags Block the device until a CAN frame is received. This
                  parameter is irrelevant for osaSocketCAN.
  */
  osaCANBus::Errno Recv( osaCANBusFrame& frame, 
			osaCANBus::Flags flags = osaCANBus::MSG_NOFLAG );
  
  //! Add a filter to the SocketCAN device
  osaCANBus::Errno AddFilter( const osaCANBus::Filter& filter );

};

#endif
