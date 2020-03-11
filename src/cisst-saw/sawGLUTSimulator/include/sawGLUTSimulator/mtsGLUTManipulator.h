/*

  Author(s): Simon Leonard
  Created on: Nov 11 2009

  (C) Copyright 2009-2011 Johns Hopkins University (JHU), All Rights Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---
*/

#ifndef _mtsGLUTManipulator_h
#define _mtsGLUTManipulator_h

#include <cisstOSAbstraction/osaCPUAffinity.h>
#include <cisstMultiTask/mtsTaskPeriodic.h>
#include <cisstParameterTypes/prmPositionJointGet.h>
#include <cisstParameterTypes/prmPositionCartesianGet.h>
#include <cisstParameterTypes/prmPositionJointSet.h>

#include <sawGLUTSimulator/osaGLUTManipulator.h>
#include <sawGLUTSimulator/sawGLUTSimulatorExport.h>

class osaMeshTriangular;

class CISST_EXPORT mtsGLUTManipulator : public mtsTaskPeriodic
{
  CMN_DECLARE_SERVICES(CMN_NO_DYNAMIC_CREATION, CMN_LOG_ALLOW_DEFAULT)

 private:

  void Initialize(const vctDoubleVec & qinit);

  osaGLUTManipulator manipulator;

  prmPositionCartesianGet Rtout;
  prmPositionJointGet qout;
  prmPositionJointSet qin;

  mtsInterfaceProvided* input;
  mtsInterfaceProvided* output;
  mtsInterfaceProvided* ctl;

  osaCPUMask cpumask;
  int priority;

 public: 

  //! GLUT Manipulator generic constructor
  /**
     This constructor initializes an GLUT manipulator with the kinematics and 
     dynamics contained in a file. Plus it initializes the GLUT elements of the
     manipulators (bodies and joints) for the engine.
     \param name The name of the component
     \param period The period of the task
     \param mask The CPU mask (not used)
     \param priority The task priority (not used)
     \param geomfiles A vector of 3D model file names
     \param Rtw0 The offset transformation of the robot base
     \param robotfn The file with the kinematics and dynamics parameters
     \param qinit The initial joint angles
     \param basefile The file name of the base 3D model
  */
  mtsGLUTManipulator( const std::string& name,
                      double period,
                      osaCPUMask mask,
                      int _priority,
                      const std::vector<std::string>& geomfiles,
                      const vctFrame4x4<double>& Rtw0,
                      const std::string& robotfn,
                      const vctDoubleVec& qinit,
                      const std::string& basefile,
                      bool rotateX90 = false );

  mtsGLUTManipulator( const std::string& name,
                      double period,
                      osaCPUMask mask,
                      int _priority,
                      const std::vector<std::string>& geomfiles,
                      const vctFrm3& Rtw0,
                      const std::string& robotfn,
                      const vctDoubleVec& qinit,
                      const std::string& basefile,
                      bool rotateX90 = false );

  ~mtsGLUTManipulator(){}

  void Configure( const std::string& CMN_UNUSED(argv) = "" ){}

  void Startup(){}
  void Run();
  void Cleanup(){}

};

CMN_DECLARE_SERVICES_INSTANTIATION(mtsGLUTManipulator);

#endif
