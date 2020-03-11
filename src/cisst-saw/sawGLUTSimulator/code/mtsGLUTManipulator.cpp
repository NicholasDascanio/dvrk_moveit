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

#include <cisstCommon/cmnLogger.h>
#include <cisstMultiTask/mtsInterfaceProvided.h>

#include <sawGLUTSimulator/mtsGLUTManipulator.h>

CMN_IMPLEMENT_SERVICES_DERIVED(mtsGLUTManipulator, mtsTaskPeriodic);

// main constructor
mtsGLUTManipulator::mtsGLUTManipulator( const std::string& name,
                      double period,
                      osaCPUMask mask,
                      int _priority,
                      const std::vector<std::string>& geomfiles,
                      const vctFrame4x4<double>& Rtw0,
                      const std::string& robotfn,
                      const vctDoubleVec& qinit,
                      const std::string& basefile,
                      bool rotateX90) :
    mtsTaskPeriodic(name, period),
    manipulator(geomfiles, Rtw0, robotfn, qinit, basefile, rotateX90),
    qout(qinit.size()),
    qin(qinit.size()),
    input(0), output(0), ctl(0),
    cpumask(mask), priority(_priority)
{
    Initialize(qinit);
}

mtsGLUTManipulator::mtsGLUTManipulator( const std::string& name,
                      double period,
                      osaCPUMask mask,
                      int _priority,
                      const std::vector<std::string>& geomfiles,
                      const vctFrm3& Rtw0,
                      const std::string& robotfn,
                      const vctDoubleVec& qinit,
                      const std::string& basefile,
                      bool rotateX90) :
    mtsTaskPeriodic(name, period),
    manipulator(geomfiles, Rtw0, robotfn, qinit, basefile, rotateX90),
    qout(qinit.size()),
    qin(qinit.size()),
    input(0), output(0), ctl(0),
    cpumask(mask), priority(_priority)
{
    Initialize(qinit);
}

void mtsGLUTManipulator::Initialize(const vctDoubleVec &qinit)
{
    qin.SetGoal(qinit);
    qout.SetPosition(qinit);
    input = AddInterfaceProvided( "Input" );
    if (input) {
        StateTable.AddData( qin, "PositionJointInput" );
        input->AddCommandWriteState( StateTable, qin, "SetPositionJoint" );
    }
    else {
        CMN_LOG_CLASS_RUN_ERROR << "Failed to create interface Input for " 
                                << GetName() << std::endl;
    }

    output = AddInterfaceProvided( "Output" );
    if (output) {
        StateTable.AddData( qout,  "PositionJointOutput" );
        StateTable.AddData( Rtout, "PositionCartesianOutput" );
        output->AddCommandReadState( StateTable, Rtout, "GetPositionCartesian" );
        output->AddCommandReadState( StateTable, qout,  "GetPositionJoint" );
    }
    else {
        CMN_LOG_CLASS_RUN_ERROR << "Failed to create interface Output for " 
                                << GetName() << std::endl;
    }
}

void mtsGLUTManipulator::Run()
{
    ProcessQueuedCommands();

    if (!manipulator.SetPositions(qin.Goal()))
        CMN_LOG_CLASS_RUN_ERROR << "Failed to get position for " << GetName() << std::endl;

    if (!manipulator.GetPositions(qout.Position()))
        CMN_LOG_CLASS_RUN_ERROR << "Failed to get position for " << GetName() << std::endl;

    vctFrame4x4<double> Rt4x4 = manipulator.ForwardKinematics(qin.Goal());
    Rtout.Position().From(Rt4x4);
}
