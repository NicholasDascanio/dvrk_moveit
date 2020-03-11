#include <cisstCommon/cmnGetChar.h>
#include <cisstCommon/cmnPath.h>
#include <cisstOSAbstraction/osaSleep.h>
#include <cisstOSAbstraction/osaGetTime.h>

#include <sawKeyboard/mtsKeyboard.h>
#include <sawBarrett/mtsWAM.h>
#include <sawControllers/mtsPDGC.h>
#include <sawTrajectories/mtsTrajectory.h>

#if (CISST_OS == CISST_LINUX_XENOMAI)
#include <native/task.h>
#include <sys/mman.h>
#include <sawCANBus/osaRTSocketCAN.h>
#else
#include <sawCANBus/osaSocketCAN.h>
#endif

class SetPoints : public mtsTaskPeriodic {

private:

  robManipulator* manipulator;

  mtsFunctionWrite SetPositionCartesian;

  vctFrame4x4<double> Rt;
  double x;
  vctDynamicVector<double> q;
  double t;

public:

  SetPoints( const std::string& robotfilename,
	     const vctFrame4x4<double>& Rtw0,
	     const vctDynamicVector<double>& qinit ) :
    mtsTaskPeriodic( "setpoint", 0.1, true ),
    manipulator( NULL ),
    q( qinit ),
    t( 0.0 ){

    manipulator = new robManipulator( robotfilename, Rtw0 );

    Rt = manipulator->ForwardKinematics( q );
    x = Rt[0][3];

    mtsInterfaceRequired* output = AddInterfaceRequired( "Output" );
    if( output ){
      output->AddFunction( "SetPositionCartesian", SetPositionCartesian );
    }
    else{
      CMN_LOG_RUN_ERROR << "Failed to create interface Output for " << GetName()
			<< std::endl;
    }

  }

  void Configure( const std::string& ){}
  void Startup(){}
  void Run(){
    ProcessQueuedCommands();

    t += GetPeriodicity();
    double dx = 0.1 * sin( t - cmnPI_2 ) + 0.1;
    Rt[0][3] = x + -dx;

    mtsFrm4x4 mtsRt( Rt );
    SetPositionCartesian( mtsRt );

  }
  void Cleanup(){}

};


int main( int argc, char** argv ){

#if (CISST_OS == CISST_LINUX_XENOMAI)
  mlockall(MCL_CURRENT | MCL_FUTURE);
  RT_TASK task;
  rt_task_shadow( &task, "mtsWAMPDGCExample", 99, 0 );
#endif

  mtsTaskManager* taskManager = mtsTaskManager::GetInstance();

  cmnLogger::SetMask( CMN_LOG_ALLOW_ALL );
  cmnLogger::SetMaskFunction( CMN_LOG_ALLOW_ALL );
  cmnLogger::SetMaskDefaultLog( CMN_LOG_ALLOW_ALL );

  if( argc != 2 ){
    std::cout << "Usage: " << argv[0] << " can[0-1]" << std::endl;
    return -1;
  }

  mtsKeyboard kb;
  kb.SetQuitKey( 'q' );
  kb.AddKeyWriteFunction( 'C', "PDGCEnable", "Enable", true );
  taskManager->AddComponent( &kb );

#if (CISST_OS == CISST_LINUX_XENOMAI)
  osaRTSocketCAN can( argv[1], osaCANBus::RATE_1000 );
#else
  osaSocketCAN can( argv[1], osaCANBus::RATE_1000 );
#endif

  if( can.Open() != osaCANBus::ESUCCESS ){
    CMN_LOG_RUN_ERROR << argv[0] << "Failed to open " << argv[1] << std::endl;
    return -1;
  }

  mtsWAM WAM( "WAM", &can, osaWAM::WAM_7DOF, OSA_CPU4, 80 );
  WAM.Configure();
  WAM.SetPositions( vctDynamicVector<double>(7,
					     0.0, -cmnPI_2, 0.0, cmnPI,
					     0.0, 0.0, 0.0 ) );
  taskManager->AddComponent( &WAM );

  cmnPath path;
  path.AddRelativeToCisstShare("/models/WAM");
  std::string fname = path.Find("wam7.rob", cmnPath::READ);

  // Rotate the base
  vctMatrixRotation3<double> Rw0(  0.0,  0.0, -1.0,
                                   0.0,  1.0,  0.0,
                                   1.0,  0.0,  0.0 );
  vctFixedSizeVector<double,3> tw0(0.0);
  vctFrame4x4<double> Rtw0( Rw0, tw0 );

  // Initial configuration
  vctDynamicVector<double> qinit( 7, 0.0 );
  qinit[1] = -cmnPI_2;
  qinit[3] =  cmnPI;

  // Gain matrices
  vctDynamicMatrix<double> Kp(7, 7, 0.0), Kd(7, 7, 0.0);
  Kp[0][0] = 250;     Kd[0][0] = 3.0;
  Kp[1][1] = 250;     Kd[1][1] = 3.0;
  Kp[2][2] = 250;     Kd[2][2] = 3.0;
  Kp[3][3] = 200;     Kd[3][3] = 3;
  Kp[4][4] = 50;      Kd[4][4] = 0.8;
  Kp[5][5] = 50;      Kd[5][5] = 0.8;
  Kp[6][6] = 10;      Kd[6][6] = .1;

  mtsPDGC PDGC( "PDGC",
		0.002,
		fname,
		Rtw0,
		Kp,
		Kd,
		qinit,
		OSA_CPU3 );
  taskManager->AddComponent( &PDGC );

  mtsTrajectory trajectory( "trajectory",
			    0.01,
			    fname,
			    Rtw0,
			    qinit );
  taskManager->AddComponent( &trajectory );

  SetPoints setpoints( fname, Rtw0, qinit );
  taskManager->AddComponent( &setpoints );

  if( !taskManager->Connect( kb.GetName(),  "PDGCEnable",
			     PDGC.GetName(),"Control") ){
    std::cout << "Failed to connect: "
	      << kb.GetName()   << "::PDGCEnable to "
	      << PDGC.GetName() << "::Control" << std::endl;
    return -1;
  }

  if( !taskManager->Connect( trajectory.GetName(), "Output",
			     PDGC.GetName(),       "Input") ){
    std::cout << "Failed to connect: "
	      << trajectory.GetName() << "::Output to "
	      << PDGC.GetName()       << "::Input" << std::endl;
    return -1;
  }

  if( !taskManager->Connect( WAM.GetName(), "Input",
			     PDGC.GetName(), "Output" ) ){
    std::cout << "Failed to connect: "
	      << WAM.GetName()  << "::Input to "
	      << PDGC.GetName() << "::Output" << std::endl;
    return -1;
  }

  if( !taskManager->Connect( WAM.GetName(),  "Output",
			     PDGC.GetName(), "Feedback" ) ){
    std::cout << "Failed to connect: "
	      << WAM.GetName()  << "::Output to "
	      << PDGC.GetName() << "::Feedback" << std::endl;
    return -1;
  }

  taskManager->CreateAll();
  taskManager->StartAll();

  pause();

  taskManager->KillAll();
  taskManager->Cleanup();

  return 0;
}
