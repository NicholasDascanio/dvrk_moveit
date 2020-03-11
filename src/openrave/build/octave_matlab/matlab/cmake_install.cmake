# Install script for directory: /home/dvrk-lite/ws_moveit_test/src/openrave/octave_matlab/matlab

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "openrave0.9-dp-matlab")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/openrave-0.9/matlab" TYPE FILE FILES
    "/home/dvrk-lite/ws_moveit_test/src/openrave/octave_matlab/orcreate.cpp"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/octave_matlab/socketconnect.h"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/octave_matlab/orread.cpp"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/octave_matlab/orwrite.cpp"
    )
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "openrave0.9-dp-matlab")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/openrave-0.9/matlab" TYPE FILE FILES
    "/home/dvrk-lite/ws_moveit_test/src/openrave/octave_matlab/orCommunicator.m"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/octave_matlab/orBodyEnable.m"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/octave_matlab/orBodyGetDOF.m"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/octave_matlab/orBodySetJointTorques.m"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/octave_matlab/orEnvDestroyProblem.m"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/octave_matlab/orBodyGetTransform.m"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/octave_matlab/orEnvCreateKinBody.m"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/octave_matlab/orRobotGetActiveDOF.m"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/octave_matlab/orRobotSensorConfigure.m"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/octave_matlab/orRobotSetDOFValues.m"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/octave_matlab/orBodyGetAABBs.m"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/octave_matlab/orEnvCheckCollision.m"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/octave_matlab/orEnvTriangulate.m"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/octave_matlab/orBodySetJointValues.m"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/octave_matlab/orRobotSensorGetData.m"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/octave_matlab/orEnvSetOptions.m"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/octave_matlab/orRobotControllerSet.m"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/octave_matlab/orSetCommunication.m"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/octave_matlab/orRobotGetDOFLimits.m"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/octave_matlab/orEnvGetRobots.m"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/octave_matlab/orEnvLoadScene.m"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/octave_matlab/orRobotCheckSelfCollision.m"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/octave_matlab/orRobotSetActiveDOFs.m"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/octave_matlab/orEnvCreateRobot.m"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/octave_matlab/orBodyGetLinks.m"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/octave_matlab/orEnvGetBody.m"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/octave_matlab/orRobotStartActiveTrajectory.m"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/octave_matlab/orEnvPlot.m"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/octave_matlab/orRobotGetDOFValues.m"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/octave_matlab/orEnvCreateProblem.m"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/octave_matlab/orEnvClose.m"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/octave_matlab/orProblemSendCommand.m"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/octave_matlab/orRobotSetActiveManipulator.m"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/octave_matlab/orEnvStepSimulation.m"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/octave_matlab/orEnvWait.m"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/octave_matlab/orBodyGetJointValues.m"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/octave_matlab/orRobotControllerSend.m"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/octave_matlab/orEnvRayCollision.m"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/octave_matlab/orRobotSensorSend.m"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/octave_matlab/orBodySetTransform.m"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/octave_matlab/orEnvCreateModule.m"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/octave_matlab/orEnvGetBodies.m"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/octave_matlab/orBodyGetAABB.m"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/octave_matlab/orEnvLoadPlugin.m"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/octave_matlab/orBodyDestroy.m"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/octave_matlab/orRobotGetManipulators.m"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/octave_matlab/orRobotGetAttachedSensors.m"
    )
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "openrave0.9-dp-matlab")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/openrave-0.9/matlab" TYPE DIRECTORY FILES "/home/dvrk-lite/ws_moveit_test/src/openrave/octave_matlab/examples" REGEX "/\\.svn$" EXCLUDE)
endif()

