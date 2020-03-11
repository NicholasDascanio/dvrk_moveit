# Install script for directory: /home/dvrk-lite/ws_moveit_test/src/openrave/plugins

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

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/home/dvrk-lite/ws_moveit_test/src/openrave/build/plugins/basecontrollers/cmake_install.cmake")
  include("/home/dvrk-lite/ws_moveit_test/src/openrave/build/plugins/baserobots/cmake_install.cmake")
  include("/home/dvrk-lite/ws_moveit_test/src/openrave/build/plugins/basesamplers/cmake_install.cmake")
  include("/home/dvrk-lite/ws_moveit_test/src/openrave/build/plugins/basesensors/cmake_install.cmake")
  include("/home/dvrk-lite/ws_moveit_test/src/openrave/build/plugins/bulletrave/cmake_install.cmake")
  include("/home/dvrk-lite/ws_moveit_test/src/openrave/build/plugins/mobyrave/cmake_install.cmake")
  include("/home/dvrk-lite/ws_moveit_test/src/openrave/build/plugins/configurationcache/cmake_install.cmake")
  include("/home/dvrk-lite/ws_moveit_test/src/openrave/build/plugins/dualmanipulation/cmake_install.cmake")
  include("/home/dvrk-lite/ws_moveit_test/src/openrave/build/plugins/grasper/cmake_install.cmake")
  include("/home/dvrk-lite/ws_moveit_test/src/openrave/build/plugins/ikfastsolvers/cmake_install.cmake")
  include("/home/dvrk-lite/ws_moveit_test/src/openrave/build/plugins/logging/cmake_install.cmake")
  include("/home/dvrk-lite/ws_moveit_test/src/openrave/build/plugins/pqprave/cmake_install.cmake")
  include("/home/dvrk-lite/ws_moveit_test/src/openrave/build/plugins/rmanipulation/cmake_install.cmake")
  include("/home/dvrk-lite/ws_moveit_test/src/openrave/build/plugins/rplanners/cmake_install.cmake")
  include("/home/dvrk-lite/ws_moveit_test/src/openrave/build/plugins/textserver/cmake_install.cmake")
  include("/home/dvrk-lite/ws_moveit_test/src/openrave/build/plugins/qtosgrave/cmake_install.cmake")
  include("/home/dvrk-lite/ws_moveit_test/src/openrave/build/plugins/qtcoinrave/cmake_install.cmake")
  include("/home/dvrk-lite/ws_moveit_test/src/openrave/build/plugins/oderave/cmake_install.cmake")
  include("/home/dvrk-lite/ws_moveit_test/src/openrave/build/plugins/fclrave/cmake_install.cmake")

endif()

