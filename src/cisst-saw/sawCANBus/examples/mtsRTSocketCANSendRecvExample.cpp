#include <cisstMultiTask/mtsTaskContinuous.h>
#include <cisstMultiTask/mtsTaskManager.h>
#include <cisstMultiTask/mtsInterfaceRequired.h>

#include <cisstCommon/cmnGetChar.h>
#include <cisstOSAbstraction/osaSleep.h>
#include <sawCANBus/mtsCANBusFrame.h>

#if (CISST_OS == CISST_LINUX_XENOMAI)
#include <sawCANBus/mtsRTSocketCAN.h>
#include <native/task.h>
#include <sys/mman.h>
#else
#include <sawCANBus/mtsSocketCAN.h>
#endif

// A client CAN task that read/write from/to a mtsCAN component
class CANclient : public mtsTaskContinuous{

private:

  mtsFunctionRead  read;    // read mts function
  mtsFunctionWrite write;   // write mts function

  mtsFunctionVoid  open;    // open the CAN device
  mtsFunctionVoid  close;   // close the CAN device

  size_t cnt;               // counter for read/write

public:

  // Constructor
  CANclient() : 
    mtsTaskContinuous( "client" ),   // task name
    cnt(0) {                         // reset counter


    // Required interface with Read/Write function
    mtsInterfaceRequired* IO = AddInterfaceRequired( "IO" );
    if( IO != NULL ){
      IO->AddFunction( "Write", write );
      IO->AddFunction( "Read", read );
    }

    // Required interface with Read/Write function
    mtsInterfaceRequired* CTL = AddInterfaceRequired( "CTL" );
    if( CTL != NULL ){
      CTL->AddFunction( "Open", open );
      CTL->AddFunction( "Close", close );
    }

  }

  ~CANclient(){}

  // Return the number of iterations
  size_t Count() const { return cnt; }
  
  void Configure(){}

  // Open the CAN device
  void Startup(){ open(); }

  void Run(){
    ProcessQueuedCommands();
    cnt++;

    // Create and send CAN a frame
    osaCANBusFrame::ID id = 1;
    vctDynamicVector<osaCANBusFrame::Data> data( 8, 1, 2, 3, 4, 5, 6, 7, cnt );
    mtsCANBusFrame framesend( id, data );
    write( framesend );
    std::cout << "Sending: " << std::endl 
	      << (osaCANBusFrame)framesend << std::endl;

    // Receive a CAN frame. If the CAN component is in loopback mode we should
    // receive the frame we just sent
    mtsCANBusFrame framerecv;
    read( framerecv );
    std::cout << "Received: " << std::endl
	      << (osaCANBusFrame)framerecv << std::endl << std::endl;

  }

  // Close the CAN device
  void Cleanup(){ close(); }

};

int main( int argc, char** argv ){

#if (CISST_OS == CISST_LINUX_XENOMAI)
  // Xenomai stuff
  mlockall(MCL_CURRENT | MCL_FUTURE);
  RT_TASK task;
  rt_task_shadow( &task, "CANServer", 80, 0);
#endif

  cmnLogger::SetMask( CMN_LOG_ALLOW_ALL );
  cmnLogger::SetMaskFunction( CMN_LOG_ALLOW_ALL );
  cmnLogger::SetMaskDefaultLog( CMN_LOG_ALLOW_ALL );

  if( argc !=2 ){
    std::cerr << "Usage: " << argv[0] << " can[?]" << std::endl;
    return -1;
  }

  // Better to have the CAN device in loopback mode
  std::cout << "Ensure that the CAN device is in loopback mode" << std::endl;

  // get local component manager                                                
  mtsManagerLocal *taskManager;
  taskManager = mtsManagerLocal::GetInstance();

  // The RTSocketCAN component
#if (CISST_OS == CISST_LINUX_XENOMAI)
  mtsRTSocketCAN can( "CAN",
		      argv[1],
		      osaCANBus::RATE_1000,
		      osaCANBus::LOOPBACK_ON );
#else
  mtsSocketCAN can( "CAN",
		    argv[1],
		    osaCANBus::RATE_1000,
		    osaCANBus::LOOPBACK_ON );
#endif
  taskManager->AddComponent( &can );
  
  // A client
  CANclient client;
  taskManager->AddComponent( &client );

  // Connect the interfaces
  taskManager->Connect( client.GetName(), "IO", can.GetName(), "IO" );
  taskManager->Connect( client.GetName(), "CTL", can.GetName(), "CTL" );
  
  taskManager->CreateAll();
  taskManager->StartAll();

  // Wait to exit
  std::cout << "ENTER to exit" << std::endl;
  cmnGetChar();

  //taskManager->KillAll();
  //taskManager->Cleanup();

  return 0;
}
