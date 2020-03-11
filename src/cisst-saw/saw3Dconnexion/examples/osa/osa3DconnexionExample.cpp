#include <cisstCommon/cmnLogger.h>
#include <saw3Dconnexion/osa3Dconnexion.h>
#include <iostream>
#include <iomanip>

int main( int argc, char** argv ){

  cmnLogger::SetMask( CMN_LOG_ALLOW_ALL );
  cmnLogger::SetMaskFunction( CMN_LOG_ALLOW_ALL );
  cmnLogger::SetMaskDefaultLog( CMN_LOG_ALLOW_ALL );

  osa3Dconnexion spacenavigator;
  if( argc == 2 ){ 
    if( spacenavigator.Open( argv[1] ) != osa3Dconnexion::ESUCCESS ){
      std::cerr << "Failed to open device " << argv[1] << std::endl;
      return -1;
    }
  }
  else{
    std::cerr << "Usage: " << argv[0] << " joystick_device_file" << std::endl;
    return -1;
  }
    
  bool button1=false, button2=false;

  std::cout << "Press both buttons to exit. " << std::endl;
  while( !button1 || !button2 ){

    osa3Dconnexion::Event event = spacenavigator.WaitForEvent();
    switch( event.type ){

    case osa3Dconnexion::Event::MOTION:
      std::cout << std::setw(10) << event.data[0]
		<< std::setw(10) << event.data[1]
		<< std::setw(10) << event.data[2]
		<< std::setw(10) << event.data[3]
		<< std::setw(10) << event.data[4]
		<< std::setw(10) << event.data[5] << std::endl;
      break;

    case osa3Dconnexion::Event::BUTTON_PRESSED:
      std::cout << "Button " << event.button << " pressed." << std::endl;
      if( event.button == osa3Dconnexion::Event::BUTTON1 )
	{ button1 = true; };
      if( event.button == osa3Dconnexion::Event::BUTTON2 )
	{ button2 = true; };
      break;

    case osa3Dconnexion::Event::BUTTON_RELEASED:
      std::cout << "Button " << event.button << " released." << std::endl;
      if( event.button == osa3Dconnexion::Event::BUTTON1 )
	{ button1 = false; };
      if( event.button == osa3Dconnexion::Event::BUTTON2 )
	{ button2 = false; };
      break;

    }
    
  }

  return 0;
}
