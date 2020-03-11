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


#include <sawCANBus/osaSocketCAN.h>
#include <cisstCommon/cmnLogger.h>


osaSocketCAN::osaSocketCAN( const std::string& devicename, 
				osaCANBus::Rate rate,
				osaCANBus::Loopback loopback ) : 
  osaCANBus( rate, loopback ),
  devicename( devicename ),
  filterscnt( 0 ){

  // Check if the device name is empty
  if( devicename.empty() )
    { CMN_LOG_RUN_WARNING << "No device name." << std::endl; }

}

osaSocketCAN::~osaSocketCAN(){}

osaCANBus::Errno osaSocketCAN::Open(){


  int errno;
  struct ifreq ifr;

  // create a CAN socket
  canfd = socket( PF_CAN, SOCK_RAW, CAN_RAW );
  if( canfd < 0 ){
    CMN_LOG_RUN_ERROR << "Couldn't create a CAN socket." << std::endl;
    return osaCANBus::EFAILURE;
  }

  if( loopback == osaCANBus::LOOPBACK_ON ){
    errno = setsockopt( canfd, 
			SOL_CAN_RAW, 
			CAN_RAW_LOOPBACK,
			&loopback, 
			sizeof(loopback) );
    if( errno != 0) {
      CMN_LOG_RUN_ERROR << "Couldn't set loopback mode for " << devicename
			<< ". Error code was: " << errno
			<< std::endl;
      return osaCANBus::EFAILURE;
    }
  }

  // Get CAN interface index by name
  strncpy(ifr.ifr_name, devicename.data(), IFNAMSIZ);
  errno = ioctl( canfd, SIOCGIFINDEX, &ifr );
  if( errno != 0 ){
    CMN_LOG_RUN_ERROR << "Couldn't get the CAN interface index by name."
		      << "Error code was: " << errno
		      << std::endl;
    return osaCANBus::EFAILURE;
  }

  //! The socket address for the CAN address family
  struct sockaddr_can addr;

  // Bind the socket to the local address
  memset(&addr, 0, sizeof(addr));     // clear the address
  addr.can_ifindex = ifr.ifr_ifindex; // ifr_ifindex was set from SIOCGIFINDEX
  addr.can_family = AF_CAN;           // Address Family CAN

  errno = bind( canfd, (struct sockaddr*)&addr, sizeof(struct sockaddr_can) );
  if( errno != 0 ){
    CMN_LOG_RUN_ERROR << "Couldn't bind the socket. Error code was: " 
		      << errno << std::endl;
    return EFAILURE;
  }

#if 0
  // set the baud rate
  can_baudrate_t* can_baudrate = (can_baudrate_t*)&ifr.ifr_ifru;
  *can_baudrate = rate;
  if( ioctl( canfd, SIOCSCANBAUDRATE, &ifr ) ){
    CMN_LOG_RUN_ERROR << "Couldn't set the rate." << std::endl;
    return EFAILURE;
  }
#endif

#if 0
  // Set the mode 
  CAN_MODE* mode = (CAN_MODE*)&ifr.ifr_ifru;
  *mode = CAN_MODE_START;
  if( rt_dev_ioctl(canfd, SIOCSCANMODE, &ifr) ){
    CMN_LOG_RUN_ERROR << "Couldn't set the operation mode." << std::endl;
    return EFAILURE;
  }
#endif
  /*
  nanosecs_rel_t timeout = 0;
  if (setsockopt( canfd, RTCAN_RTIOC_SND_TIMEOUT, &timeout ) ){
    perror("osaSocketCAN::open: Couldn't set the send timeout: ");
    return EFAILURE;
  }
  
  if( setsockopt( canfd, RTCAN_RTIOC_RCV_TIMEOUT, &timeout ) ){
    perror("osaSocketCAN::open: Couldn't set the recv timeout: ");
    return EFAILURE;
  }
  */

  return ESUCCESS;
}

osaCANBus::Errno osaSocketCAN::Close(){
  // close the socket
  if( close( canfd ) ){
    CMN_LOG_RUN_ERROR << "Couldn't close the socket." << std::endl;
    return EFAILURE;
  }
  return ESUCCESS;
}

// Send a can frame
// Note that block is useless for Socket CAN
osaCANBus::Errno osaSocketCAN::Send( const osaCANBusFrame& canframe, 
				      osaCANBus::Flags ){
  // copy the data in to a Socket CAN frame
  // can_frame_t is defined in xenomai/include/rtdm/rtcan.h
  can_frame frame;
  frame.can_id = (canid_t)canframe.GetID();
  frame.can_dlc = (__u8)canframe.GetLength();  

  const __u8* data = (const __u8*)canframe.GetData();
  for(size_t i=0; i<8; i++)
    { frame.data[i] = data[i]; }

  // send the frame
  int error = send( canfd, (void*)&frame, sizeof(can_frame), 0 );
  if( error < 0 ){
    perror( "error: " );
    CMN_LOG_RUN_ERROR << "Failed to send CAN frame " << error << std::endl;
    return EFAILURE;
  }

  return ESUCCESS;
}

// Receive a CAN frame
osaCANBus::Errno osaSocketCAN::Recv(osaCANBusFrame& canframe, osaCANBus::Flags){
  struct can_frame frame;            // the RT Socket CAN frame
  memset(&frame, 0, sizeof(frame));  // clear the frame
  
  int error = recv( canfd, (void*)&frame, sizeof(can_frame), 0 );
  if( error < 0 ){
    perror( "error: " );
    CMN_LOG_RUN_ERROR << "Failed to receive the frame. Error: " << error 
		      << std::endl;
    return EFAILURE;
  }

  // create a osaCANBusFrame
  canframe = osaCANBusFrame( frame.can_id, frame.data, frame.can_dlc );

  return ESUCCESS;
}

osaCANBus::Errno osaSocketCAN::AddFilter( const osaCANBus::Filter& filter ){

  //std::cout << "osaSocketCAN::AddFilter" << std::endl;

  if( filterscnt < MAX_NUM_FILTERS ){

    /*
    // Avoid duplicates
    for( size_t i=0; i<filterscnt; i++ ){
      if( filters[i].can_mask == filter.mask && filters[i].can_id == filter.id )
	{ return osaCANBus::ESUCCESS; }
    }
    */

    filters[filterscnt].can_mask = filter.mask;
    filters[filterscnt].can_id   = filter.id;
    filterscnt++;

    // Set the filter to the socket
    if( setsockopt( canfd, 
		    SOL_CAN_RAW, 
		    CAN_RAW_FILTER, 
		    filters, 
		    filterscnt*sizeof(struct can_filter) ) ){
      CMN_LOG_RUN_ERROR << "Couldn't set the socket filters." << std::endl;
      return osaCANBus::EFAILURE;
    }

    return osaCANBus::ESUCCESS;
  }

  else{
    CMN_LOG_RUN_ERROR << "Reached maximum number of filters." << std::endl;
    return osaCANBus::EFAILURE;
  }

  return osaCANBus::EFAILURE;

}



