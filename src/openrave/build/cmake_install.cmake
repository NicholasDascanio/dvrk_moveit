# Install script for directory: /home/dvrk-lite/ws_moveit_test/src/openrave

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

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "openrave0.9-dp-base")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE PROGRAM FILES "/home/dvrk-lite/ws_moveit_test/src/openrave/build/openrave0.9-config")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "openrave")
  execute_process(COMMAND /usr/bin/cmake -E make_directory ${CMAKE_INSTALL_PREFIX}/bin COMMAND /usr/bin/cmake -E make_directory ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig COMMAND /usr/bin/cmake -E make_directory ${CMAKE_INSTALL_PREFIX}/)
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  
            if ("$ENV{DESTDIR}" STREQUAL "")
                execute_process(COMMAND "/usr/bin/cmake" -E create_symlink
                                /usr/local/bin/openrave0.9-config
                                /usr/local/bin/openrave-config)
            else ()
                execute_process(COMMAND "/usr/bin/cmake" -E create_symlink
                                /usr/local/bin/openrave0.9-config
                                $ENV{DESTDIR}//usr/local/bin/openrave-config)
            endif ()
        
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "openrave0.9-dp-dev")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/openrave-0.9" TYPE FILE FILES
    "/home/dvrk-lite/ws_moveit_test/src/openrave/build/openrave-config.cmake"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/build/openrave-config-version.cmake"
    )
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "openrave0.9-dp-base")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/openrave-0.9" TYPE FILE FILES "/home/dvrk-lite/ws_moveit_test/src/openrave/build/openrave_completion.bash")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "openrave0.9-dp-dev")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/home/dvrk-lite/ws_moveit_test/src/openrave/build/openrave0.9.pc")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "openrave0.9-dp-dev")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/home/dvrk-lite/ws_moveit_test/src/openrave/build/openrave0.9-core.pc")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  
            if ("$ENV{DESTDIR}" STREQUAL "")
                execute_process(COMMAND "/usr/bin/cmake" -E create_symlink
                                /usr/local/lib/pkgconfig/openrave0.9.pc
                                /usr/local/lib/pkgconfig/openrave.pc)
            else ()
                execute_process(COMMAND "/usr/bin/cmake" -E create_symlink
                                /usr/local/lib/pkgconfig/openrave0.9.pc
                                $ENV{DESTDIR}//usr/local/lib/pkgconfig/openrave.pc)
            endif ()
        
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  
            if ("$ENV{DESTDIR}" STREQUAL "")
                execute_process(COMMAND "/usr/bin/cmake" -E create_symlink
                                /usr/local/lib/pkgconfig/openrave0.9-core.pc
                                /usr/local/lib/pkgconfig/openrave-core.pc)
            else ()
                execute_process(COMMAND "/usr/bin/cmake" -E create_symlink
                                /usr/local/lib/pkgconfig/openrave0.9-core.pc
                                $ENV{DESTDIR}//usr/local/lib/pkgconfig/openrave-core.pc)
            endif ()
        
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "openrave0.9-dp-base")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/openrave-0.9" TYPE FILE FILES "/home/dvrk-lite/ws_moveit_test/src/openrave/build/openrave.bash")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "openrave0.9-dp-dev")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/openrave-0.9/openrave" TYPE FILE FILES
    "/home/dvrk-lite/ws_moveit_test/src/openrave/include/openrave/sensor.h"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/include/openrave/viewer.h"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/include/openrave/iksolver.h"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/include/openrave/plugininfo.h"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/include/openrave/kinbody.h"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/include/openrave/plannerparameters.h"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/include/openrave/openrave.h"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/include/openrave/controller.h"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/include/openrave/geometry.h"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/include/openrave/logging.h"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/include/openrave/xmlreaders.h"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/include/openrave/physicsengine.h"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/include/openrave/environment.h"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/include/openrave/sensorsystem.h"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/include/openrave/utils.h"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/include/openrave/mathextra.h"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/include/openrave/plugin.h"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/include/openrave/module.h"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/include/openrave/collisionchecker.h"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/include/openrave/robot.h"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/include/openrave/interface.h"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/include/openrave/spacesampler.h"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/include/openrave/trajectory.h"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/include/openrave/planningutils.h"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/include/openrave/planner.h"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/build/include/openrave/interfacehashes.h"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/build/include/openrave/config.h"
    )
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "openrave0.9-dp-cbindings-dev")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/openrave-0.9/openrave_c" TYPE FILE FILES "/home/dvrk-lite/ws_moveit_test/src/openrave/include/openrave_c/openrave_c.h")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "openrave0.9-dp-dev")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/openrave-0.9/rave" TYPE FILE FILES
    "/home/dvrk-lite/ws_moveit_test/src/openrave/rave/rave.h"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/rave/plugin.h"
    )
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "openrave0.9-dp-base")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/openrave-0.9" TYPE FILE FILES
    "/home/dvrk-lite/ws_moveit_test/src/openrave/COPYING"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/LICENSE.lgpl"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/LICENSE.apache"
    )
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/home/dvrk-lite/ws_moveit_test/src/openrave/build/cpp-gen-md5/cmake_install.cmake")
  include("/home/dvrk-lite/ws_moveit_test/src/openrave/build/3rdparty/crlibm-1.0beta4/cmake_install.cmake")
  include("/home/dvrk-lite/ws_moveit_test/src/openrave/build/3rdparty/ivcon/cmake_install.cmake")
  include("/home/dvrk-lite/ws_moveit_test/src/openrave/build/3rdparty/ann/cmake_install.cmake")
  include("/home/dvrk-lite/ws_moveit_test/src/openrave/build/3rdparty/fparser-4.5/cmake_install.cmake")
  include("/home/dvrk-lite/ws_moveit_test/src/openrave/build/3rdparty/convexdecomposition/cmake_install.cmake")
  include("/home/dvrk-lite/ws_moveit_test/src/openrave/build/src/cmake_install.cmake")
  include("/home/dvrk-lite/ws_moveit_test/src/openrave/build/octave_matlab/cmake_install.cmake")
  include("/home/dvrk-lite/ws_moveit_test/src/openrave/build/locale/cmake_install.cmake")
  include("/home/dvrk-lite/ws_moveit_test/src/openrave/build/python/cmake_install.cmake")
  include("/home/dvrk-lite/ws_moveit_test/src/openrave/build/plugins/cmake_install.cmake")

endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/home/dvrk-lite/ws_moveit_test/src/openrave/build/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
