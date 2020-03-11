#include <cisstCommon/cmnGetChar.h>
#include <cisstCommon/cmnPath.h>

#include <cisstMultiTask/mtsTaskManager.h>
#include <cisstMultiTask/mtsTaskPeriodic.h>
#include <cisstMultiTask/mtsInterfaceRequired.h>

#include <sawOpenSceneGraph/mtsOSGMono.h>
#include <sawOpenDynamicsEngine/mtsODEWorld.h>
#include <sawOpenDynamicsEngine/mtsODEManipulator.h>

class WAMMotion : public mtsTaskPeriodic {

private:

  mtsFunctionRead  GetPositions;
  mtsFunctionWrite SetPositions;

  vctDynamicVector<double> q;

public:

  WAMMotion( double period ) :
    mtsTaskPeriodic( "WAMMotion", period, true ){

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

int main(){

  mtsTaskManager* taskManager = mtsTaskManager::GetInstance();

  cmnLogger::SetMask( CMN_LOG_ALLOW_ALL );
  cmnLogger::SetMaskFunction( CMN_LOG_ALLOW_ALL );
  cmnLogger::SetMaskDefaultLog( CMN_LOG_ALLOW_ALL );

  osg::ref_ptr< mtsODEWorld > world = NULL;
  world = new mtsODEWorld( "world", 0.00001 );
  taskManager->AddComponent( world.get() );

  // Create a camera
  int x = 0, y = 0;
  int width = 640, height = 480;
  double Znear = 0.1, Zfar = 10.0;
  mtsOSGMono* camera;
  camera = new mtsOSGMono( "camera",
			   world,
			   x, y, width, height,
			   55.0, ((double)width)/((double)height),
			   Znear, Zfar );
  taskManager->AddComponent( camera );


  cmnPath path;
  path.AddRelativeToCisstShare("/models/WAM");
  vctFrame4x4<double> Rtw0;

  std::vector< std::string > models;
  models.push_back( path.Find("l1.obj") );
  models.push_back( path.Find("l2.obj") );
  models.push_back( path.Find("l3.obj") );
  models.push_back( path.Find("l4.obj") );
  models.push_back( path.Find("l5.obj") );
  models.push_back( path.Find("l6.obj") );
  models.push_back( path.Find("l7.obj") );

  mtsODEManipulator* WAM;
  WAM = new mtsODEManipulator( "WAM",
			       0.001,
			       OSA_CPU1,
			       20,
			       models,
			       world,
			       Rtw0,
			       path.Find("wam7.rob"),
			       path.Find("l0.obj"),
			       vctDynamicVector<double>( 7, 0.0 ) );
  taskManager->AddComponent( WAM );

  WAMMotion motion( 0.001 );
  taskManager->AddComponent( &motion );

  taskManager->Connect( motion.GetName(), "Input",  WAM->GetName(), "Output" );
  taskManager->Connect( motion.GetName(), "Output", WAM->GetName(), "Input" );

  taskManager->CreateAll();
  taskManager->WaitForStateAll( mtsComponentState::READY );

  taskManager->StartAll();
  taskManager->WaitForStateAll( mtsComponentState::ACTIVE );

  cmnGetChar();
  std::cout << "ENTER to quit" << std::endl;
  cmnGetChar();

  taskManager->KillAll();
  taskManager->Cleanup();

  return 0;

}
