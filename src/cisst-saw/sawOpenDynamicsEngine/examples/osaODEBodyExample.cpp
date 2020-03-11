#include <cisstCommon/cmnPath.h>
#include <sawOpenSceneGraph/osaOSGMono.h>
#include <sawOpenDynamicsEngine/osaODEWorld.h>
#include <sawOpenDynamicsEngine/osaODEBody.h>

int main(){

  cmnLogger::SetMask( CMN_LOG_ALLOW_ALL );
  cmnLogger::SetMaskFunction( CMN_LOG_ALLOW_ALL );
  cmnLogger::SetMaskDefaultLog( CMN_LOG_ALLOW_ALL );

  // Create the world
  osaODEWorld* world = new osaODEWorld( 0.001 );

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
  path.AddRelativeToCisstShare("/models");
  path.AddRelativeToCisstShare("/models/hubble");

  // Create a rigid body. Make up some mass + com + moit
  double mass = 1.0;
  vctFixedSizeVector<double,3> com( 0.0 );
  vctFixedSizeMatrix<double,3,3> moit = vctFixedSizeMatrix<double,3,3>::Eye();

  vctFixedSizeVector<double,3> u( 0.780004, 0.620257, 0.082920 );
  u.NormalizedSelf();
  vctFrame4x4<double> Rtwh( vctAxisAngleRotation3<double>( u, 0.7391 ),
			    vctFixedSizeVector<double,3>( 0.0, 0.0, 1.0 ) );
  osg::ref_ptr<osaODEBody> hubble;
  hubble = new osaODEBody( path.Find("hst.3ds"), world, Rtwh, mass, com, moit );


  // Create a static body. This body has no mass and cannot be moved
  osg::ref_ptr<osaODEBody> background;
  background = new osaODEBody( path.Find("background.3ds"), world, vctFrm3() );

  std::cout << "ESC to quit" << std::endl;
  while( !camera->done() ){

    world->Step();
    camera->frame();

  }

  return 0;

}
