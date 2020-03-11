#include <cisstCommon/cmnPath.h>
#include <sawOpenDynamicsEngine/osaODEWorld.h>
#include <sawOpenDynamicsEngine/osaODEBarrettHand.h>
#include <sawOpenDynamicsEngine/osaODEManipulator.h>
#include <sawOpenSceneGraph/osaOSGMono.h>

int main(){

  cmnLogger::SetMask( CMN_LOG_ALLOW_ALL );
  cmnLogger::SetMaskFunction( CMN_LOG_ALLOW_ALL );
  cmnLogger::SetMaskDefaultLog( CMN_LOG_ALLOW_ALL );

  // Create the OSG World
  osaODEWorld* world = new osaODEWorld( 0.00001 );

  // Create a camera
  int x = 0, y = 0;
  int width = 640, height = 480;
  double Znear = 0.01, Zfar = 10.0;
  osg::ref_ptr<osaOSGMono> camera;
  camera = new osaOSGMono( world,
			   x, y, width, height,
			   55, ((double)width)/((double)height),
			   Znear, Zfar );
  camera->Initialize();

  // Create objects
  cmnPath wampath;
  wampath.AddRelativeToCisstShare("/models/WAM");

  // Create a rigid body. Make up some mass + com + moit
  double mass = 1.0;
  vctFixedSizeVector<double,3> com( 0.0 );
  vctFixedSizeMatrix<double,3,3> moit = vctFixedSizeMatrix<double,3,3>::Eye();

  cmnPath hubblepath;
  hubblepath.AddRelativeToCisstShare("/models/hubble");
  vctFixedSizeVector<double,3> u( 0.780004, 0.620257, 0.082920 );
  u.NormalizedSelf();
  vctFrame4x4<double> Rtwh( vctAxisAngleRotation3<double>( u, 0.7391 ),
			    vctFixedSizeVector<double,3>( 0.5, 0.5, 1.0 ) );
  osg::ref_ptr<osaODEBody> hubble;
  hubble = new osaODEBody( hubblepath.Find("hst.3ds"), world, Rtwh, mass, com, moit );


  std::vector< std::string > wammodels;
  wammodels.push_back( wampath.Find("l1.obj") );
  wammodels.push_back( wampath.Find("l2.obj") );
  wammodels.push_back( wampath.Find("l3.obj") );
  wammodels.push_back( wampath.Find("l4.obj") );
  wammodels.push_back( wampath.Find("l5.obj") );
  wammodels.push_back( wampath.Find("l6.obj") );
  wammodels.push_back( wampath.Find("l7.obj") );

  osg::ref_ptr<osaODEManipulator> wam;
  wam = new osaODEManipulator( wammodels,
			       world,
			       vctFrame4x4<double>(),
			       wampath.Find("wam7.rob"),
			       wampath.Find("l0.obj"),
			       vctDynamicVector<double>( 7, 0.0 ) );


  cmnPath bhpath;
  bhpath.AddRelativeToCisstShare("/models/BH");
  vctFrame4x4<double> Rtw0 = wam->ForwardKinematics( vctDynamicVector<double>( 7, 0.0 ) );
  osg::ref_ptr<osaODEBarrettHand> bh;

  bh = new osaODEBarrettHand( bhpath.Find("l0.obj"),
			      bhpath.Find("l1.obj"),
			      bhpath.Find("l2.obj"),
			      bhpath.Find("l3.obj"),
			      world,
			      Rtw0,
			      bhpath.Find("f1f2.rob"),
			      bhpath.Find("f3.rob") );
  wam->Attach( bh.get() );

  std::cout << "ESC to quit" << std::endl;

  vctDynamicVector<double> qwam( 7, 0.0 );
  vctDynamicVector<double> qbh( 4, 0.0 );

  while( !camera->done() ){

    for( size_t i=0; i<7; i++ ) qwam[i] += 0.005;
    for( size_t i=0; i<4; i++ ) qbh[i] += 0.005;
    wam->SetPositions( qwam );
    bh->SetPositions( qbh );
    world->Step();
    camera->frame();

  }

  return 0;
}
