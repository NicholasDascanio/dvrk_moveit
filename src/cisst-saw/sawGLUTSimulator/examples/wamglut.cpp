/*

  Author(s): Simon Leonard

  (C) Copyright 2009-2011 Johns Hopkins University (JHU), All Rights Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---
*/

#include <cisstCommon/cmnPath.h>
#include <cisstMultiTask/mtsComponent.h>
#include <cisstMultiTask/mtsManagerLocal.h>
#include <cisstMultiTask/mtsInterfaceProvided.h>
#include <cisstMultiTask/mtsInterfaceRequired.h>
#include <cisstParameterTypes/prmPositionJointSet.h>
#include <sawGLUTSimulator/osaGLUT.h>
#include <sawGLUTSimulator/mtsGLUTManipulator.h>
#include <sawKeyboard/mtsKeyboard.h>

class SetPoints : public mtsComponent {
  mtsFunctionWrite WriteSetPoint;
  prmPositionJointSet startPos;
  prmPositionJointSet endPos;
  void NextSetPoint(const mtsBool &val);
public:
   SetPoints(const std::string &name, unsigned int numAxes);
   ~SetPoints() {}
   void SetEndpoints(const vctDoubleVec &start, const vctDoubleVec &end);
};

SetPoints::SetPoints(const std::string &name, unsigned int numAxes) : mtsComponent(name),
                     startPos(numAxes), endPos(numAxes)
{
  mtsInterfaceProvided *prov = AddInterfaceProvided("Control");
  if (prov)
    prov->AddCommandWrite(&SetPoints::NextSetPoint, this, "NextSetPoint", mtsBool());
  mtsInterfaceRequired *req = AddInterfaceRequired("Output");
  if (req)
    req->AddFunction("SetPositionJoint", WriteSetPoint);
}

void SetPoints::NextSetPoint(const mtsBool &val)
{
  if (val) {
    CMN_LOG_RUN_VERBOSE << "Writing start setpoint " << startPos << std::endl;
    WriteSetPoint(startPos);
  }
  else {
    CMN_LOG_RUN_VERBOSE << "Writing end setpoint " << endPos << std::endl;
    WriteSetPoint(endPos);
  }
}

void SetPoints::SetEndpoints(const vctDoubleVec &start, const vctDoubleVec &end)
{
  startPos.SetGoal(start);
  startPos.SetValid(true);
  endPos.SetGoal(end);
  endPos.SetValid(true);
}

int main( int argc, char** argv ){

  cmnLogger::SetMask( CMN_LOG_ALLOW_ALL );
  cmnLogger::SetMaskFunction( CMN_LOG_ALLOW_ALL );
  cmnLogger::SetMaskDefaultLog( CMN_LOG_ALLOW_ALL );

  mtsManagerLocal* taskManager = mtsManagerLocal::GetInstance();

  osaGLUT glut(argc, argv);

  mtsKeyboard keyboard;
  keyboard.SetQuitKey('q');
  keyboard.AddKeyWriteFunction( 'n', "next", "NextSetPoint", false );
  taskManager->AddComponent( &keyboard );

  vctDoubleVec qinit( 7, 0.0 );
  vctDoubleVec qfinal( 7, 1.0 );

  SetPoints setpoints( "setpoints", 7 );
  setpoints.SetEndpoints( qinit, qfinal );
  taskManager->AddComponent(&setpoints);

#if 0
  vctDynamicVector<double> qdmax( 7, 0.1 );
  vctDynamicVector<double> qddmax( 7, 0.05 );
  devQLQRn trajectory( "trajectory",
                       0.01,
                       devTrajectory::ENABLED,
                       OSA_CPUANY,
                       devTrajectory::QUEUE,
                       devTrajectory::POSITION,
                       qinit,
                       qdmax,
                       qddmax );
  taskManager->AddComponent(&trajectory);
#endif

  std::string path;
  if (!cmnPath::GetCisstShare(path)) {
    CMN_LOG_RUN_ERROR << "Could not find cisst share directory -- is CISST_ROOT defined?" << std::endl;
    return -1;
  }
  path.append("/models/WAM/");
  std::vector<std::string> links;

  links.push_back( path + "l1.obj" );
  links.push_back( path + "l2.obj" );
  links.push_back( path + "l3.obj" );
  links.push_back( path + "l4.obj" );
  links.push_back( path + "l5.obj" );
  links.push_back( path + "l6.obj" );
  links.push_back( path + "l7.obj" );

  mtsGLUTManipulator WAM("WAM",
                         0.03,
                         OSA_CPUANY,
                         0,
                         links,
                         vctFrame4x4<double>(),
                         path + "wam7.rob",
                         qinit,
                         path + "l0.obj",
                         true );
  taskManager->AddComponent(&WAM);

  if (!taskManager->Connect( keyboard.GetName(),  "next",
                             setpoints.GetName(), "Control" ) )
    {
      std::cerr << "Connect failed: "
                << keyboard.GetName() << ":next"
                << " - "
                << setpoints.GetName() << ":Control"
                << std::endl;
      return 1;
    }

#if 0
  if (!taskManager->Connect( trajectory.GetName(), devTrajectory::Input,
                             setpoints.GetName(),  devSetPoints::OutputRn ) )
    {
      std::cerr << "Connect failed: "
                << trajectory.GetName() << ":" << devTrajectory::Input
                << " - "
                << setpoints.GetName() << ":" << devManipulator::Input
                << std::endl;
      return 1;
    }

  if (!taskManager->Connect( trajectory.GetName(), devTrajectory::Output,
                             WAM.GetName(),    devManipulator::Input ))
    {
      std::cerr << "Connect failed: "
                << trajectory.GetName() << ":" << "Output"
                << " - "
                << WAM.GetName() << ":" << devManipulator::Input
                << std::endl;
      return 1;
    }
#else
  if (!taskManager->Connect( setpoints.GetName(), "Output",
                             WAM.GetName(),    "Input" ))
    {
      std::cerr << "Connect failed: "
                << setpoints.GetName() << ":Output"
                << " - "
                << WAM.GetName() << ":Input"
                << std::endl;
      return 1;
    }
#endif


  taskManager->CreateAll();
  taskManager->StartAll();

  std::cout << "Type 'n' to move to the final position" << std::endl;
  std::cout << "Type 'n' again to move to the initial position" << std::endl;

  osaGLUT::StartMainLoop();

  return 0;

}
