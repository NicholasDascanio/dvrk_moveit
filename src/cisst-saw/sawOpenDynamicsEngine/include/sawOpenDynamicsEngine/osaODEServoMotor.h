
#ifndef _osaODEServoMotor_h
#define _osaODEServoMotor_h

#include <ode/ode.h>

#include <cisstVector/vctFixedSizeVector.h>
#include <sawOpenDynamicsEngine/sawOpenDynamicsEngineExport.h>

class CISST_EXPORT osaODEServoMotor {

 private:

  dJointID motorid;

  double vwmax;
  double ftmax;

 public:

  osaODEServoMotor( dWorldID world, 
		    dBodyID body1, 
		    dBodyID body2,
		    const vctFixedSizeVector<double,3>& axis,
		    double vwmax,
		    double ftmax,
		    dJointType motortype );

  dJointID MotorID() const;

  void SetPosition( double qs, double q, double dt );
  void SetVelocity( double qd );

};

#endif

