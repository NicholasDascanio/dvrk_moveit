#include <cisstCommon/cmnPath.h>
#include <sawOpenDynamicsEngine/osaODEWorld.h>
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
  cmnPath path;
  path.AddRelativeToCisstShare("/models/WAM");
  path.AddRelativeToCisstShare("/models/hubble");

  // Create a rigid body. Make up some mass + com + moit
  double mass = 1.0;
  vctFixedSizeVector<double,3> com( 0.0 );
  vctFixedSizeMatrix<double,3,3> moit = vctFixedSizeMatrix<double,3,3>::Eye();

  vctFixedSizeVector<double,3> u( 0.780004, 0.620257, 0.082920 );
  u.NormalizedSelf();
  vctFrame4x4<double> Rtwh( vctAxisAngleRotation3<double>( u, 0.7391 ),
			    vctFixedSizeVector<double,3>( 0.5, 0.5, 1.0 ) );
  osg::ref_ptr<osaODEBody> hubble;
  hubble = new osaODEBody( path.Find("hst.3ds"), world, Rtwh, mass, com, moit );


  std::vector< std::string > models;
  models.push_back( path.Find("l1.obj") );
  models.push_back( path.Find("l2.obj") );
  models.push_back( path.Find("l3.obj") );
  models.push_back( path.Find("l4.obj") );
  models.push_back( path.Find("l5.obj") );
  models.push_back( path.Find("l6.obj") );
  models.push_back( path.Find("l7.obj") );

  osg::ref_ptr<osaODEManipulator> wam;
  wam = new osaODEManipulator( models,
			       world,
			       vctFrame4x4<double>(),
			       path.Find("wam7.rob"),
			       path.Find("l0.obj"),
			       vctDynamicVector<double>( 7, 0.0 ) );

  std::cout << "ESC to quit" << std::endl;

  vctDynamicVector<double> q( 7, 0.0 );
  while( !camera->done() ){

    for( size_t i=0; i<7; i++ ) q[i] += 0.00005;
    wam->SetPositions( q );
    world->Step();
    camera->frame();

  }

  return 0;
}
