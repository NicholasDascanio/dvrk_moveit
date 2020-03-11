#include <cisstCommon/cmnPath.h>
#include <cisstCommon/cmnGetChar.h>

#include <cisstMultiTask/mtsTaskManager.h>
#include <cisstMultiTask/mtsTaskPeriodic.h>
#include <cisstMultiTask/mtsInterfaceRequired.h>

#include <sawOpenDynamicsEngine/mtsODEWorld.h>
#include <sawOpenSceneGraph/mtsOSGMono.h>
#include <sawOpenDynamicsEngine/mtsODEBarrettHand.h>

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

  osg::ref_ptr< mtsODEWorld > world = NULL;
  world = new mtsODEWorld( "world", 0.0001, vctFixedSizeVector<double,3>(0.0) );
  taskManager->AddComponent( world.get() );

  // Create a camera
  int x = 0, y = 0;
  int width = 640, height = 480;
  double Znear = 0.1, Zfar = 10.0;
  mtsOSGMono* camera;
  camera = new mtsOSGMono( "camera",
                           world,
                           x, y,
                           width, height,
                           55.0, ((double)width)/((double)height),
                           Znear, Zfar );
  taskManager->AddComponent( camera );

  cmnPath path;
  path.AddRelativeToCisstShare("/models/BH");
  std::string l0 = path.Find("l0.rob", cmnPath::READ);
  std::string l1 = path.Find("l1.rob", cmnPath::READ);
  std::string l2 = path.Find("l2.rob", cmnPath::READ);
  std::string l3 = path.Find("l3.rob", cmnPath::READ);
  std::string f1f2 = path.Find("f1f2.rob", cmnPath::READ);
  std::string f3 = path.Find("f3.rob", cmnPath::READ);

  vctFrame4x4<double> Rtw0;
  Rtw0[2][3] = 0.1;

  mtsODEBarrettHand* BH;
  BH = new mtsODEBarrettHand( "BH",
			      0.001,
			      OSA_CPU1,
			      20,
			      l0,
			      l1,
			      l2,
			      l3,
			      world,
			      Rtw0,
			      f1f2,
			      f3 );
  taskManager->AddComponent( BH );

  BHMotion motion;
  taskManager->AddComponent( &motion );

  taskManager->Connect( motion.GetName(), "Input",  BH->GetName(), "Output" );
  taskManager->Connect( motion.GetName(), "Output", BH->GetName(), "Input" );

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
