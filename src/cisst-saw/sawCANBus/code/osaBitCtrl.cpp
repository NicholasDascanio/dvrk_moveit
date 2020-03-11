
#include <sawCANBus/osaBitCtrl.h>
#include <cisstCommon/cmnLogger.h>
#include <cisstOSAbstraction/osaSleep.h>

#include <can.h>
#include <fcntl.h> // for O_RDWR

CMN_IMPLEMENT_SERVICES( osaBitCtrl );

osaBitCtrl::osaBitCtrl( const std::string& candevname, osaCANBus::Rate rate ) : 
  osaCANBus( rate ),
  candevname( candevname ),
  canfd( -1 ){}

osaBitCtrl::~osaBitCtrl(){
  
  // ensure the device is closed
  if( Close() == osaCANBus::EFAILURE ){
    CMN_LOG_RUN_ERROR << " Failed to close device " << candevname
		      << std::endl;
  }
  
}

osaCANBus::Errno osaBitCtrl::Open(){

#if (CISST_OS == CISST_QNX )

  // ensure the device is not already opened
  if( IsClosed() ){
    
    // open the device
    canfd = open( candevname.data(), O_RDWR );
    
    // check the file descriptor
    if( IsClosed() ){
      CMN_LOG_INIT_ERROR << " Failed to open the CAN device " << candevname
			 << std::endl;
      return osaCANBus::EFAILURE;
    }
    
    // SL: duno what this is supposed to do
    ioctl( canfd, CNFLUSH );
    return osaCANBus::ESUCCESS;
    
  }
  
  else{
    CMN_LOG_RUN_ERROR << "The CAN device has already been opened?"
		      << std::endl;
    return osaCANBus::EFAILURE;
  }
#endif

  return osaCANBus::EFAILURE;
}

osaCANBus::Errno osaBitCtrl::Close(){
  
#if (CISST_OS == CISST_QNX )

  // ensure the device is opened
  if( IsOpened() ){
    
    // close the device
    if( close( canfd ) == -1 ){
      CMN_LOG_RUN_ERROR << " Failed to close the device " << candevname 
			<< std::endl;
      return osaCANBus::EFAILURE;
    }
    // reset the file descriptor
    canfd = -1;
    
    return osaCANBus::ESUCCESS;
  }
#endif

  return osaCANBus::EFAILURE;

}

osaCANBus::Errno osaBitCtrl::Recv( osaCANBusFrame& frame, osaCANBus::Flags ){

#if (CISST_OS == CISST_QNX )

  // ensure the device is opened
  if( IsOpened() ){
    
    // the can message
    canmsg_t canmsg;
    
    // read the message
    int nbytesread;

    nbytesread = read( canfd, &canmsg, sizeof(canmsg_t) );

    // check the nuber of bytes
    if( nbytesread != sizeof( canmsg_t ) ){
      CMN_LOG_RUN_ERROR << " Expected to read " << sizeof( canmsg_t ) << " bytes."
			<< " Got " << nbytesread
			<< std::endl;
      return osaCANBus::EFAILURE;	
    }
    
    // build and return a CAN frame
    frame = osaCANBusFrame( canmsg.id, canmsg.data, canmsg.length );
    return osaCANBus::ESUCCESS;
    
  }

  else{
    CMN_LOG_RUN_ERROR << "Invalid file descriptor. Is the CAN deviced opened?"
		      << std::endl;
    return osaCANBus::EFAILURE;
  }

#endif

  return osaCANBus::EFAILURE;
  
}

osaCANBus::Errno osaBitCtrl::Send( const osaCANBusFrame& frame, osaCANBus::Flags ){
  
#if (CISST_OS == CISST_QNX )

  // ensure the device is opened
  if( IsOpened() ){
    
    // the can message
    canmsg_t canmsg;
    
    // copy the values
    canmsg.flags = 0;
    canmsg.id = frame.GetID();
    canmsg.length = frame.GetLength();
    const osaCANBusFrame::Data* data = frame.GetData();
    for( osaCANBusFrame::DataLength i=0; i<frame.GetLength(); i++ )
      { canmsg.data[i] = data[i]; }
    
    // write the message
    int nbyteswrite;
    nbyteswrite = write( canfd, &canmsg, sizeof(canmsg_t) );
    
    // check the number of bytes
    if( nbyteswrite != sizeof( canmsg_t ) ){
      CMN_LOG_RUN_ERROR << " Expected to write " << sizeof( canmsg_t ) << " bytes."
			<< " Wrote " << nbyteswrite
			<< std::endl;
      return osaCANBus::EFAILURE;	
    }
    
    return osaCANBus::ESUCCESS;
    
  }
  
  else{
    CMN_LOG_RUN_ERROR << "Invalid file descriptor. Is the CAN deviced opened?"
		      << std::endl;
    return osaCANBus::EFAILURE;
  } 

#endif

  return osaCANBus::EFAILURE;
  
}

