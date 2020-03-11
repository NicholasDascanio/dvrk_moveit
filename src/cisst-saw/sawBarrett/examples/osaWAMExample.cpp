#include <cisstOSAbstraction/osaSleep.h>
#include <cisstOSAbstraction/osaGetTime.h>

#include <sawBarrett/osaWAM.h>
#include <sawCANBus/osaSocketCAN.h>

int main( int argc, char** argv ){

  cmnLogger::SetMask( CMN_LOG_ALLOW_ALL );
  cmnLogger::SetMaskFunction( CMN_LOG_ALLOW_ALL );
  cmnLogger::SetMaskDefaultLog( CMN_LOG_ALLOW_ALL );

  if( argc != 2 ){
    std::cout << "Usage: " << argv[0] << " can[0-1]" << std::endl;
    return -1;
  }

  osaSocketCAN can( argv[1], osaCANBus::RATE_1000 );

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

  double t1 = osaGetTime();
  size_t cnt=0;

  while( 1 ){

    vctDynamicVector<double> q;
    if( WAM.GetPositions( q ) != osaWAM::ESUCCESS ){
      CMN_LOG_RUN_ERROR << "Failed to get positions" << std::endl;
      return -1;
    }

    vctDynamicVector<double> tau( q.size(), 0.0 );
    if( WAM.SetTorques( tau ) != osaWAM::ESUCCESS ){
      CMN_LOG_RUN_ERROR << "Failed to set torques" << std::endl;
      return -1;
    }

    std::cout << "q: " << q << std::endl;
    cnt++;
    if( cnt == 1000 ){
      double t2 = osaGetTime();
      std::cout << 1000.0 / (t2 - t1) << std::endl;
      t1 = t2;
      cnt = 0;
    }

  }

  return 0;
}
