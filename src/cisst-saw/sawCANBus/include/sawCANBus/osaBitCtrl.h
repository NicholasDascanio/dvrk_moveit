

#ifndef _osaBitCtrl_h
#define _osaBitCtrl_h

#include <sawCANBus/osaCANBus.h>
#include <sawCANBus/sawCANBusExport.h>

//! A BitCtrl CAN device
/**
   This implements a CAN device based on the BitCtrl driver. It's not really a 
   driver since it only provides an API and uses ISO C read/write.
*/

class CISST_EXPORT osaBitCtrl : public osaCANBus {
  
  CMN_DECLARE_SERVICES(CMN_NO_DYNAMIC_CREATION, CMN_LOG_LOD_RUN_ERROR);

 private:
  
  std::string candevname;
  int canfd;

  bool IsOpened() const { return canfd != -1 ? true : false; }
  bool IsClosed() const { return canfd == -1 ? true : false; }
   
 public:
  
  
  osaBitCtrl( const std::string& candevname, osaCANBus::Rate rate );
  ~osaBitCtrl();
  
  osaCANBus::Errno Open();
  osaCANBus::Errno Close();
  
  osaCANBus::Errno Send( const osaCANBusFrame& frame, 
			osaCANBus::Flags flags = osaCANBus::MSG_NOFLAG );
  
  osaCANBus::Errno Recv( osaCANBusFrame& frame, 
			osaCANBus::Flags flags = osaCANBus::MSG_NOFLAG );
  
  osaCANBus::Errno AddFilter( const osaCANBus::Filter& ) 
    { return osaCANBus::ESUCCESS; }
  
  
};

CMN_DECLARE_SERVICES_INSTANTIATION(osaBitCtrl)

#endif
