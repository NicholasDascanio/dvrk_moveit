#include <cisstCommon/cmnPath.h>
#include <sawOpenDynamicsEngine/osaODEWorld.h>
#include <sawOpenSceneGraph/osaOSGMono.h>
#include <sawOpenDynamicsEngine/osaODEBarrettHand.h>

int main(){

  cmnLogger::SetMask( CMN_LOG_ALLOW_ALL );
  cmnLogger::SetMaskFunction( CMN_LOG_ALLOW_ALL );
  cmnLogger::SetMaskDefaultLog( CMN_LOG_ALLOW_ALL );

  osg::ref_ptr< osaODEWorld > world = new osaODEWorld(0.001, vctFixedSizeVector<double,3>(0.0));


  // Create a camera
  int x = 0, y = 0;
  int width = 640, height = 480;
  double Znear = 0.1, Zfar = 10.0;
  osg::ref_ptr< osaOSGCamera > camera;
  camera = new osaOSGMono( world,
			     x, y, width, height,
			     55.0, ((double)width)/((double)height),
			     Znear, Zfar );
  camera->Initialize();

  cmnPath pathbh;
  pathbh.AddRelativeToCisstShare("/models/BH");
  vctFrame4x4<double> Rtw0;
  Rtw0[2][3] = 0.1;

  osg::ref_ptr<osaODEBarrettHand> bh;
  bh = new osaODEBarrettHand( pathbh.Find("l0.obj"),
			      pathbh.Find("l1.obj"),
			      pathbh.Find("l2.obj"),
			      pathbh.Find("l3.obj"),
			      world,
			      Rtw0,
			      pathbh.Find("f1f2.rob"),
			      pathbh.Find("f3.rob") );

  std::cout << "ESC to quit" << std::endl;

  vctDynamicVector<double> q( 4, 0.0 );
  while( !camera->done() ){

    for( size_t i=0; i<4; i++ ) q[i] += 0.001;
    bh->SetPositions( q );
    world->Step();
    camera->frame();

  }

  return 0;

}
