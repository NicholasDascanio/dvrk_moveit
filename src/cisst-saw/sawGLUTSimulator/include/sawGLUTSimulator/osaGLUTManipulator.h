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

#ifndef _osaGLUTManipulator_h
#define _osaGLUTManipulator_h

#include <cisstVector/vctDynamicVectorTypes.h>
#include <cisstRobot/robManipulator.h>

#include <sawGLUTSimulator/sawGLUTSimulatorExport.h>

class osaMeshTriangular;

class CISST_EXPORT osaGLUTManipulator : public robManipulator
{
 protected:

  //! Store the current joints values
  vctDoubleVec q;

  osaMeshTriangular* base;
  std::vector< osaMeshTriangular* > meshes;

 public: 

  //! GLUT Manipulator generic constructor
  /**
     This constructor initializes a GLUT manipulator with the kinematics and 
     dynamics contained in a file. Plus it initializes the GLUT elements of the
     manipulators (bodies and joints) for the engine.
     \param geomfiles A vector of 3D model file names
     \param Rtw0 The offset transformation of the robot base
     \param robotfn The file with the kinematics and dynamics parameters
     \param qinit The initial joint angles
     \param basefile The file name of the base 3D model
  */
  osaGLUTManipulator( const std::vector<std::string>& geomfiles,
                      const vctFrame4x4<double>& Rtw0,
                      const std::string& robotfn,
                      const vctDoubleVec& qinit,
                      const std::string& basefile,
                      bool rotateX90 = false );

  //! GLUT Manipulator generic constructor
  /**
     This constructor initializes a GLUT manipulator with the kinematics and 
     dynamics contained in a file. Plus it initializes the GLUT elements of the
     manipulators (bodies and joints) for the engine.
     \param geomfiles A vector of 3D model file names
     \param Rtw0 The offset transformation of the robot base
     \param robotfn The file with the kinematics and dynamics parameters
     \param qinit The initial joint angles
     \param basefile The file name of the base 3D model
  */
  osaGLUTManipulator( const std::vector<std::string>& geomfiles,
                      const vctFrm3& Rtw0,
                      const std::string& robotfn,
                      const vctDoubleVec& qinit,
                      const std::string& basefile,
                      bool rotateX90 = false );

  ~osaGLUTManipulator();

  void Initialize(const std::vector<std::string>& geomfiles,
                  const std::string& basefile, bool rotateX90);

  //! Return the joints positions
  /**
     Query each joint and return the joint positions
     \param q[in] A vector of joints positions
     \return true if successfull. false otherwise.
  */
  virtual bool GetPositions(vctDoubleVec& q) const;

  //! Set the joint position
  /**
     This sets the position command and stores a local copy
     \param q A vector of joint positions
     \return true if successfull. false otherwise.
  */
  virtual bool SetPositions(const vctDoubleVec& q);

};

#endif
