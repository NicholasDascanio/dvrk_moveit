#include <cisstCommon/cmnGetChar.h>
#include <cisstCommon/cmnPath.h>
#include <cisstOSAbstraction/osaSleep.h>
#include <cisstOSAbstraction/osaGetTime.h>

#include <sawBarrett/osaWAM.h>
#include <sawControllers/osaPDGC.h>

#if (CISST_OS == CISST_LINUX_XENOMAI)
#include <sawCANBus/osaRTSocketCAN.h>
#include <native/task.h>
#include <sys/mman.h>
#else
#include <sawCANBus/osaSocketCAN.h>
#endif

int main( int argc, char** argv ){

#if (CISST_OS == CISST_LINUX_XENOMAI)
  mlockall(MCL_CURRENT | MCL_FUTURE);
  RT_TASK task;
  rt_task_shadow( &task, "GroupTest", 99, 0 );
#endif

  cmnLogger::SetMask( CMN_LOG_ALLOW_ALL );
  cmnLogger::SetMaskFunction( CMN_LOG_ALLOW_ALL );
  cmnLogger::SetMaskDefaultLog( CMN_LOG_ALLOW_ALL );

  if( argc != 2 ){
    std::cout << "Usage: " << argv[0] << " can[0-1]" << std::endl;
    return -1;
  }

#if (CISST_OS == CISST_LINUX_XENOMAI)
  osaRTSocketCAN can( argv[1], osaCANBus::RATE_1000 );
#else
  osaSocketCAN can( argv[1], osaCANBus::RATE_1000 );
#endif

  if( can.Open() != osaCANBus::ESUCCESS ){
    CMN_LOG_RUN_ERROR << argv[0] << "Failed to open " << argv[1] << std::endl;
    return -1;
  }

  osaWAM WAM( &can );

  if( WAM.Initialize() != osaWAM::ESUCCESS ){
    CMN_LOG_RUN_ERROR << "Failed to initialize WAM" << std::endl;
    return -1;
  }

  vctDynamicVector<double> qinit( 7, 0.0 );
  qinit[1] = -cmnPI_2;
  qinit[3] =  cmnPI;

  if( WAM.SetPositions( qinit ) != osaWAM::ESUCCESS ){
    CMN_LOG_RUN_ERROR << "Failed to set position: " << qinit << std::endl;
    return -1;
  }

  cmnPath path;
  path.AddRelativeToCisstShare("/models/WAM");
  std::string fname = path.Find("wam7.rob", cmnPath::READ);

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

  osaPDGC PDGC( fname, Rtw0, Kp, Kd, qinit );

  std::cout << "Activate the WAM" << std::endl;
  bool activated = false;

  double t1 = osaGetTime();

  while( 1 ){

    // Get the positions
    vctDynamicVector<double> q;
    if( WAM.GetPositions( q ) != osaWAM::ESUCCESS ){
      CMN_LOG_RUN_ERROR << "Failed to get positions" << std::endl;
      return -1;
    }

    // Check if the pucks are activated
    if( !activated ) {
      osaWAM::Mode mode;
      if( WAM.GetMode( mode ) != osaWAM::ESUCCESS ){
	CMN_LOG_RUN_ERROR << "Failed to get mode" << std::endl;
	return -1;
      }
      if( mode == osaWAM::MODE_ACTIVATED )
	{ activated = true; }
    }

    // if pucks are activated, run the controller
    vctDynamicVector<double> tau( q.size(), 0.0 );
    double t2 = osaGetTime();
    if( activated ){
      if( PDGC.Evaluate( qinit, q, tau, t2-t1 ) != osaPDGC::ESUCCESS ){
	CMN_LOG_RUN_ERROR << "Failed to evaluate controller" << std::endl;
	return -1;
      }
    }
    t1 = t2;

    // apply torques
    if( WAM.SetTorques( tau ) != osaWAM::ESUCCESS ){
      CMN_LOG_RUN_ERROR << "Failed to set torques" << std::endl;
      return -1;
    }

    std::cout << "q:   " << q << std::endl;
    std::cout << "tau: " << tau << std::endl;

  }

  return 0;
}
