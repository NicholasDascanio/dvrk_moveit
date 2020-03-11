/*
  Author(s): Simon Leonard
  Created on: Dec 02 2009

  (C) Copyright 2009 Johns Hopkins University (JHU), All Rights
  Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---
*/

#ifndef _mtsODEBarrettHand_h
#define _mtsODEBarrettHand_h

#include <sawOpenDynamicsEngine/mtsODEManipulatorTask.h>
#include <sawOpenDynamicsEngine/osaODEBarrettHand.h>
#include <sawOpenDynamicsEngine/sawOpenDynamicsEngineExport.h>

class CISST_EXPORT mtsODEBarrettHand : public mtsODEManipulatorTask {

 public:

  mtsODEBarrettHand( const std::string& name,
		     double period,
		     osaCPUMask cpumask,
		     int priority,
		     const std::string& palmmodel,
		     const std::string& metacarpalmodel,
		     const std::string& proximalmodel,
		     const std::string& intermediatemodel,
		     osaODEWorld* world,
		     const vctFrame4x4<double>& Rtw0,
		     const std::string& f1f2filename,
		     const std::string& f3filename ) :
  mtsODEManipulatorTask( name, period, 
			 new osaODEBarrettHand( palmmodel,
						metacarpalmodel,
						proximalmodel,
						intermediatemodel,
						world,
						Rtw0,
						f1f2filename,
						f3filename ),
			 cpumask, priority ){}
  
  // main constructor
  mtsODEBarrettHand( const std::string& name,
		     double period,
		     osaCPUMask cpumask,
		     int priority,
		     const std::string& palmmodel,
		     const std::string& metacarpalmodel,
		     const std::string& proximalmodel,
		     const std::string& intermediatemodel,
		     osaODEWorld* world,
		     const vctFrm3& Rtw0,
		     const std::string& f1f2filename,
		     const std::string& f3filename ) :
  mtsODEManipulatorTask( name, period, 
			 new osaODEBarrettHand( palmmodel,
						metacarpalmodel,
						proximalmodel,
						intermediatemodel,
						world,
						Rtw0,
						f1f2filename,
						f3filename ),
			 cpumask, priority ){}
  
};

#endif





