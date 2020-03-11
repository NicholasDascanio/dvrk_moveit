#include <cisstCommon/cmnPath.h>
#include <cisstCommon/cmnGetChar.h>

#include <cisstMultiTask/mtsTaskManager.h>
#include <cisstMultiTask/mtsTaskPeriodic.h>
#include <cisstMultiTask/mtsInterfaceRequired.h>

#include <sawOpenDynamicsEngine/mtsODEWorld.h>
#include <sawOpenDynamicsEngine/mtsODEBarrettHand.h>
#include <sawOpenDynamicsEngine/mtsODEManipulator.h>
#include <sawOpenSceneGraph/mtsOSGMono.h>

class WAMMotion : public mtsTaskPeriodic {

private:

  mtsFunctionRead  GetPositions;
  mtsFunctionWrite SetPositions;

  vctDynamicVector<double> q;

public:

  WAMMotion() : mtsTaskPeriodic( "WAMMotion", 0.001, true ){

    q.SetSize(7);
    q.SetAll(0.0);

    mtsInterfaceRequired* input = AddInterfaceRequired( "Input" );
    mtsInterfaceRequired* output = AddInterfaceRequired( "Output" );

    input->AddFunction( "GetPositionJoint", GetPositions );
    output->AddFunction( "SetPositionJoint", SetPositions );

  }

  void Configure( const std::string& ){}
  void Startup(){}
  void Run(){
    ProcessQueuedCommands();

    prmPositionJointGet qin;
    GetPositions( qin );

    prmPositionJointSet qout;
    qout.SetSize( 7 );
    qout.Goal() = q;
    SetPositions( qout );

    for( size_t i=0; i<q.size(); i++ ) q[i] += 0.001;

  }

  void Cleanup(){}

};

class BHMotion : public mtsTaskPeriodic {

private:

  mtsFunctionRead  GetPositions;
  mtsFunctionWrite SetPositions;

  vctDynamicVector<double> q;

public:

  BHMotion() : mtsTaskPeriodic( "BHMotion", 0.001, true ){

    q.SetSize(4);
    q.SetAll(0.0);

    mtsInterfaceRequired* input = AddInterfaceRequired( "Input" );
    mtsInterfaceRequired* output = AddInterfaceRequired( "Output" );

    input->AddFunction( "GetPositionJoint", GetPositions );
    output->AddFunction( "SetPositionJoint", SetPositions );

  }

  void Configure( const std::string& ){}
  void Startup(){}
  void Run(){
    ProcessQueuedCommands();

    prmPositionJointGet qin;
    GetPositions( qin );

    prmPositionJointSet qout;
    qout.SetSize( 4 );
    qout.Goal() = q;
    SetPositions( qout );

    for( size_t i=0; i<q.size(); i++ ) q[i] += 0.001;

  }

  void Cleanup(){}

};

int main(){

  mtsTaskManager* taskManager = mtsTaskManager::GetInstance();

  cmnLogger::SetMask( CMN_LOG_ALLOW_ALL );
  cmnLogger::SetMaskFunction( CMN_LOG_ALLOW_ALL );
  cmnLogger::SetMaskDefaultLog( CMN_LOG_ALLOW_ALL );

  // Create the OSG World
  mtsODEWorld* world = new mtsODEWorld( "world", 0.00001 );
  taskManager->AddComponent( world );

  // Create a camera
  int x = 0, y = 0;
  int width = 640, height = 480;
  double Znear = 0.01, Zfar = 10.0;
  mtsOSGMono* camera;
  camera = new mtsOSGMono( "camera",
			   world,
			   x, y, width, height,
			   55, ((double)width)/((double)height),
			   Znear, Zfar );
  taskManager->AddComponent( camera );


  // Create objects
  cmnPath wampath;
  wampath.AddRelativeToCisstShare("/models/WAM");
  std::string fname;

  // Create a rigid body. Make up some mass + com + moit
  double mass = 1.0;
  vctFixedSizeVector<double,3> com( 0.0 );
  vctFixedSizeMatrix<double,3,3> moit = vctFixedSizeMatrix<double,3,3>::Eye();

  cmnPath hubblepath;
  hubblepath.AddRelativeToCisstShare("/models/hubble");
  vctFixedSizeVector<double,3> u( 0.780004, 0.620257, 0.082920 );
  u.NormalizedSelf();
  vctFrame4x4<double> Rtwh( vctAxisAngleRotation3<double>( u, 0.7391 ),
			    vctFixedSizeVector<double,3>( 0.0, 0.5, 1.0 ) );
  osg::ref_ptr<osaODEBody> hubble;
  hubble = new osaODEBody( hubblepath.Find("hst.3ds"), world, Rtwh, mass, com, moit );


  std::vector< std::string > wammodels;
  fname = wampath.Find("l1.obj");
  wammodels.push_back( fname );
  fname = wampath.Find("l2.obj");
  wammodels.push_back( fname );
  fname = wampath.Find("l3.obj");
  wammodels.push_back( fname );
  fname = wampath.Find("l4.obj");
  wammodels.push_back( fname );
  fname = wampath.Find("l5.obj");
  wammodels.push_back( fname );
  fname = wampath.Find("l6.obj");
  wammodels.push_back( fname );
  fname = wampath.Find("l7.obj");
  wammodels.push_back( fname );

  mtsODEManipulator* WAM;
  WAM = new mtsODEManipulator( "WAM",
			       0.001,
			       OSA_CPU1,
			       20,
			       wammodels,
			       world,
			       vctFrame4x4<double>(),
			       wampath.Find("wam7.rob"),
			       wampath.Find("l0.obj"),
			       vctDynamicVector<double>( 7, 0.0 ) );
  taskManager->AddComponent( WAM );

  robManipulator robwam( wampath.Find("wam7.rob"), vctFrame4x4<double>() );
  cmnPath bhpath;
  bhpath.AddRelativeToCisstShare("/models/BH");
  vctFrame4x4<double> Rtw0 = robwam.ForwardKinematics( vctDynamicVector<double>( 7, 0.0 ) );
  mtsODEBarrettHand* BH;
  BH = new mtsODEBarrettHand( "BH",
			      0.001,
			      OSA_CPU1,
			      20,
			      bhpath.Find("l0.obj"),
			      bhpath.Find("l1.obj"),
			      bhpath.Find("l2.obj"),
			      bhpath.Find("l3.obj"),
			      world,
			      Rtw0,
			      bhpath.Find("f1f2.rob"),
			      bhpath.Find("f3.rob") );
  taskManager->AddComponent( BH );

  WAM->Attach( BH );

  WAMMotion wammotion;
  taskManager->AddComponent( &wammotion );

  taskManager->Connect( wammotion.GetName(), "Input",  WAM->GetName(), "Output" );
  taskManager->Connect( wammotion.GetName(), "Output", WAM->GetName(), "Input" );

  BHMotion bhmotion;
  taskManager->AddComponent( &bhmotion );

  taskManager->Connect( bhmotion.GetName(), "Input",  BH->GetName(), "Output" );
  taskManager->Connect( bhmotion.GetName(), "Output", BH->GetName(), "Input" );

  taskManager->CreateAll();
  taskManager->WaitForStateAll( mtsComponentState::READY );

  taskManager->StartAll();
  taskManager->WaitForStateAll( mtsComponentState::ACTIVE );

  cmnGetChar();
  std::cout << "ESC to quit" << std::endl;
  cmnGetChar();

  taskManager->KillAll();
  taskManager->Cleanup();

  return 0;
}
