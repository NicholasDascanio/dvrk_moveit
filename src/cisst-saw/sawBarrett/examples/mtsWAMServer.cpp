#include <cisstCommon/cmnPath.h>
#include <cisstCommon/cmnGetChar.h>

#include <cisstOSAbstraction/osaSleep.h>
#include <cisstOSAbstraction/osaGetTime.h>

#include <cisstMultiTask/mtsInterfaceRequired.h>

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


  vctFrame4x4<double> Rt;
  double x;
  vctDynamicVector<double> q;
  vctDynamicVector<double> qready;
  double t;

  mtsInterfaceProvided* ctl;
  mtsBool mtsEnabled;
  bool online;

  mtsInterfaceProvided*  input;
  mtsInterfaceProvided* outputSE3;
  mtsInterfaceRequired* output;

  prmPositionCartesianGet prmCommand;   // Cartesian commands
  prmPositionCartesianGet prmTelemetry; // Cartesian telemetry

  mtsFunctionWrite SetPositionCartesian;

  bool IsEnabled() const { return mtsEnabled; }

public:

  SetPoints( const std::string& robotfilename,
	     const vctFrame4x4<double>& Rtw0,
	     const vctDynamicVector<double>& qinit ) :
    mtsTaskPeriodic( "Robot", 0.01, true ),
    manipulator( NULL ),
    q( qinit ),
    qready( qinit ),
    t( 0.0 ),
    ctl( NULL ),
    mtsEnabled( false ),
    online( false ),
    input( NULL ),
    output( NULL ){

    manipulator = new robManipulator( robotfilename, Rtw0 );

    Rt = manipulator->ForwardKinematics( q );
    x = Rt[0][3];

    qready[0] = 0.0;
    qready[1] = -cmnPI/2.0;
    qready[2] = 0.0;
    qready[3] =  cmnPI/2.0;
    qready[4] = 0.0;
    qready[5] = -cmnPI/2.0;
    qready[6] = 0.0;

    // The control interface: to change the state of the component
    ctl = AddInterfaceProvided( "Control" );
    if( ctl ){
      StateTable.AddData( mtsEnabled, "Enabled" );
      ctl->AddCommandWriteState( StateTable, mtsEnabled, "Enable" );
    }
    else{
      CMN_LOG_RUN_ERROR << "Failed to create interface Control for "<< GetName()
			<< std::endl;
    }

    input = AddInterfaceProvided( "Input" );
    if( input ){
      StateTable.AddData( prmCommand, "Enabled" );
      input->AddCommandWriteState(StateTable,prmCommand,"SetPositionCartesian");
    }
    else{
      CMN_LOG_RUN_ERROR << "Failed to create interface Input for " << GetName()
			<< std::endl;
    }
   
    outputSE3 = AddInterfaceProvided( "OutputSE3" );
    if( outputSE3 ){
      StateTable.AddData( prmTelemetry, "Telemetry" );
      outputSE3->AddCommandReadState( StateTable,
				      prmTelemetry,
				      "GetPositionCartesian" );
    }

    output = AddInterfaceRequired( "Output" );
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

    if( IsEnabled() ){

      if( !online ){

	online = true;

	for( size_t i=0; i<qready.size(); i++ ){

	  if( 0.015 < fabs( q[i] - qready[i] ) ){

	    online = false;

	    if( q[i] < qready[i] )
	      { q[i] += 0.0005; }
	    else if( qready[i] < q[i] )
	      { q[i] -= 0.0005; }

	  }

	}

	std::cout << q << std::endl;
	// this goes to the ikin
	mtsFrm4x4 mtsRt( manipulator->ForwardKinematics( q ) );
	SetPositionCartesian( mtsRt );

	// this goes to the teleop
	prmTelemetry.Position().FromRaw( mtsRt );
	prmTelemetry.SetValid( true );

	if( online )
	  { std::cout << "System is online. 'q' to quit" << std::endl; }

      }
      else{

	bool valid = false;
	prmCommand.GetValid( valid );

	if( valid ){

	  vctFrm3 frm3Rt;
	  prmCommand.GetPosition( frm3Rt );
	  
	  // this goes to the ikin
	  vctQuaternionRotation3<double> R( frm3Rt.Rotation(), VCT_NORMALIZE );
	  mtsFrm4x4 mtsRt( vctFrame4x4<double>( R, frm3Rt.Translation() ) );
	  SetPositionCartesian( mtsRt );
	  
	  // return the same value to the telop
	  prmTelemetry.Position().FromRaw( mtsRt );
	  prmTelemetry.SetValid( true );
	}

      }

    }

  }
  void Cleanup(){}

};


int main( int argc, char** argv ){

#if (CISST_OS == CISST_LINUX_XENOMAI)
  mlockall(MCL_CURRENT | MCL_FUTURE);
  RT_TASK task;
  rt_task_shadow( &task, "mtsWAMPDGCExample", 40, 0 );
#endif

  cmnLogger::SetMask( CMN_LOG_ALLOW_ALL );
  cmnLogger::SetMaskFunction( CMN_LOG_ALLOW_ALL );
  cmnLogger::SetMaskDefaultLog( CMN_LOG_ALLOW_ALL );

  if( argc != 3 ){
    std::cout << "Usage: " << argv[0] << " can[0-1] GCMIP" << std::endl;
    return -1;
  }

  std::string process( "Slave" );
  mtsManagerLocal* taskManager = NULL;

  try{ taskManager = mtsTaskManager::GetInstance( argv[2], process ); }
  catch( ... ){
    std::cerr << "Failed to connect to GCM: " << argv[2] << std::endl;
    taskManager = mtsManagerLocal::GetInstance();
  }

  mtsKeyboard kb;
  kb.SetQuitKey( 'q' );
  kb.AddKeyWriteFunction( 'C', "PDGCEnable", "Enable", true );
  kb.AddKeyWriteFunction( 'C', "TrajEnable", "Enable", true );
  kb.AddKeyWriteFunction( 'M', "MoveEnable", "Enable", true );
  taskManager->AddComponent( &kb );

  // Initial configuration
  vctDynamicVector<double> qinit( 7, 0.0 );
  qinit[1] = -cmnPI_2;
  qinit[3] =  cmnPI-0.01;  
  qinit[5] = -cmnPI_2;

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
  WAM.SetPositions( qinit );
  taskManager->AddComponent( &WAM );

  cmnPath path;
  path.AddRelativeToCisstShare("models/WAM");

  // Rotate the base
  vctMatrixRotation3<double> Rw0(  0.0,  0.0, -1.0,
                                   0.0,  1.0,  0.0,
                                   1.0,  0.0,  0.0 );
  vctFixedSizeVector<double,3> tw0(0.0);
  vctFrame4x4<double> Rtw0( Rw0, tw0 );

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
		0.00125,
		path.Find( "wam7cutter.rob" ),
		Rtw0,
		Kp,
		Kd,
		qinit,
		OSA_CPU3 );
  taskManager->AddComponent( &PDGC );

  mtsTrajectory traj( "trajectory",
		      0.002,
		      path.Find( "wam7cutter.rob" ), 
		      Rtw0, 
		      qinit );
  taskManager->AddComponent( &traj );

  SetPoints setpoints( path.Find( "wam7cutter.rob" ), Rtw0, qinit );
  taskManager->AddComponent( &setpoints );

  // Connect the keyboard
  if( !taskManager->Connect( kb.GetName(),  "PDGCEnable",
			     PDGC.GetName(),"Control") ){
    std::cout << "Failed to connect: "
	      << kb.GetName()   << "::PDGCEnable to "
	      << PDGC.GetName() << "::Control" << std::endl;
    return -1;
  }

  if( !taskManager->Connect( kb.GetName(),  "TrajEnable",
			     traj.GetName(),"Control") ){
    std::cout << "Failed to connect: "
	      << kb.GetName()   << "::TrajEnable to "
	      << traj.GetName() << "::Control" << std::endl;
    return -1;
  }

  if( !taskManager->Connect( kb.GetName(),       "MoveEnable",
			     setpoints.GetName(),"Control") ){
    std::cout << "Failed to connect: "
	      << kb.GetName()        << "::TrajEnable to "
	      << setpoints.GetName() << "::Control" << std::endl;
    return -1;
  }



  // connect the trajectory with setpoints
  if( !taskManager->Connect( traj.GetName(),      "Input",
			     setpoints.GetName(), "Output" ) ){
    std::cout << "Failed to connect: "
	      << traj.GetName()      << "::Input to "
	      << setpoints.GetName() << "::Output" << std::endl;
    return -1;
  }



  // connect the trajectory to the controller
  if( !taskManager->Connect( traj.GetName(), "Output",
			     PDGC.GetName(), "Input") ){
    std::cout << "Failed to connect: "
	      << traj.GetName() << "::Output to "
	      << PDGC.GetName() << "::Input" << std::endl;
    return -1;
  }



  // connect the controller to the WAM
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

  std::cout << "Press 'q' to exit." << std::endl;
  pause();

  return 0;
}
