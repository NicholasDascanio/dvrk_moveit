
#if (CISST_OS == CISST_LINUX_XENOMAI)
#include <sawCANBus/osaRTSocketCAN.h>
#include <native/task.h>
#include <sys/mman.h>
#else
#include <sawCANBus/osaSocketCAN.h>
#endif

int main( int argc, char** argv ){

#if (CISST_OS == CISST_LINUX_XENOMAI)
  mlockall(MCL_CURRENT | MCL_FUTURE);
  RT_TASK task;
  rt_task_shadow( &task, "RTSocketCANSend", 1, 0);
#endif

  cmnLogger::SetMask( CMN_LOG_ALLOW_ALL );
  cmnLogger::SetMaskFunction( CMN_LOG_ALLOW_ALL );
  cmnLogger::SetMaskDefaultLog( CMN_LOG_ALLOW_ALL );

  if( argc != 2 ){
    std::cerr << "Usage: " << argv[0] << " can[?]" << std::endl;
    return -1;
  }

#if (CISST_OS == CISST_LINUX_XENOMAI)
  osaRTSocketCAN can( argv[1], 
		      osaCANBus::RATE_1000,
		      osaCANBus::LOOPBACK_ON );
#else
  osaSocketCAN can( argv[1], 
		    osaCANBus::RATE_1000,
		    osaCANBus::LOOPBACK_ON );
#endif

  if( can.Open() != osaCANBus::ESUCCESS ){
    std::cerr << argv[0] << ": Failed to open device " << argv[1] << std::endl;
    return -1;
  }

  osaCANBusFrame::ID id = 0;
  vctDynamicVector<osaCANBusFrame::Data> data( 8, 1, 2, 3, 4, 5, 6, 7, 8 );
  osaCANBusFrame frame( id, data );
  std::cout << frame << std::endl;

  if( can.Send( frame ) != osaCANBus::ESUCCESS ){
    std::cerr << argv[0] << ": Failed to send on: " << argv[1] << std::endl;
    return -1;
  }

  if( can.Close() != osaCANBus::ESUCCESS ){
    std::cerr << argv[0] << ": Failed to close device " << argv[1] << std::endl;
    return -1;
  }


  return 0;
}
