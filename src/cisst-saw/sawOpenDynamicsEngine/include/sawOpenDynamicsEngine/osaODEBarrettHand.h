

#ifndef _osaODEBarrettHand_h
#define _osaODEBarrettHand_h

#include <sawOpenSceneGraph/osaOSGBH.h>
#include <sawOpenDynamicsEngine/osaODEWorld.h>
#include <sawOpenDynamicsEngine/sawOpenDynamicsEngineExport.h>


//! ODE Barrett Hand
/**
   This class implements a Barrett hand device for ODE simulation. The class
   is derived from osaODEManipulator yet it reimplements most of the virtual 
   methods due to the parallel and underactuated architecture.
   The hand creates 3 fingers, themselves ODE manipulators devices and 
   dispatches the I/O to each finger.
*/
class CISST_EXPORT osaODEBarrettHand : public osaOSGBH {

 public:

  //! Barrett Hand constructor
  osaODEBarrettHand( const std::string& palmmodel,
		     const std::string& metacarpalmodel,
		     const std::string& proximalmodel,
		     const std::string& intermediatemodel,
		     osaODEWorld* world,
		     const vctFrame4x4<double>& Rtw0,
		     const std::string& f1f2filename,
		     const std::string& f3filename );
  
  //! Barrett Hand constructor  
  osaODEBarrettHand( const std::string& palmmodel,
		     const std::string& metacarpalmodel,
		     const std::string& proximalmodel,
		     const std::string& intermediatemodel,
		     osaODEWorld* world,
		     const vctFrm3& Rtw0,
		     const std::string& f1f2filename,
		     const std::string& f3filename );
  
  ~osaODEBarrettHand();
  
};

#endif
