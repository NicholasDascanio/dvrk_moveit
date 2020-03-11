# Install script for directory: /home/dvrk-lite/ws_moveit_test/src/openrave/src/libopenrave-core

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
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libopenrave0.9-core.so.0.9.0"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libopenrave0.9-core.so.0"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libopenrave0.9-core.so"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      file(RPATH_CHECK
           FILE "${file}"
           RPATH "/usr/local/lib:/usr/local/lib/openrave0.9-plugins:/usr/lib/lib/x86_64-linux-gnu")
    endif()
  endforeach()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES
    "/home/dvrk-lite/ws_moveit_test/src/openrave/build/src/libopenrave-core/libopenrave0.9-core.so.0.9.0"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/build/src/libopenrave-core/libopenrave0.9-core.so.0"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/build/src/libopenrave-core/libopenrave0.9-core.so"
    )
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libopenrave0.9-core.so.0.9.0"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libopenrave0.9-core.so.0"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libopenrave0.9-core.so"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      file(RPATH_CHANGE
           FILE "${file}"
           OLD_RPATH "/usr/lib/lib/x86_64-linux-gnu:/home/dvrk-lite/ws_moveit_test/src/openrave/build/src/libopenrave:"
           NEW_RPATH "/usr/local/lib:/usr/local/lib/openrave0.9-plugins:/usr/lib/lib/x86_64-linux-gnu")
      if(CMAKE_INSTALL_DO_STRIP)
        execute_process(COMMAND "/usr/bin/strip" "${file}")
      endif()
    endif()
  endforeach()
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "openrave0.9-dp-dev")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/openrave-0.9" TYPE FILE FILES "/home/dvrk-lite/ws_moveit_test/src/openrave/src/libopenrave-core/openrave-core.h")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "openrave0.9-dp-cbindings-base")
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libopenrave0.9-core_c.so.0.9.0"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libopenrave0.9-core_c.so.0"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libopenrave0.9-core_c.so"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      file(RPATH_CHECK
           FILE "${file}"
           RPATH "/usr/local/lib:/usr/local/lib/openrave0.9-plugins:/usr/lib/lib/x86_64-linux-gnu")
    endif()
  endforeach()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES
    "/home/dvrk-lite/ws_moveit_test/src/openrave/build/src/libopenrave-core/libopenrave0.9-core_c.so.0.9.0"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/build/src/libopenrave-core/libopenrave0.9-core_c.so.0"
    "/home/dvrk-lite/ws_moveit_test/src/openrave/build/src/libopenrave-core/libopenrave0.9-core_c.so"
    )
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libopenrave0.9-core_c.so.0.9.0"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libopenrave0.9-core_c.so.0"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libopenrave0.9-core_c.so"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      file(RPATH_CHANGE
           FILE "${file}"
           OLD_RPATH "/usr/lib/lib/x86_64-linux-gnu:/home/dvrk-lite/ws_moveit_test/src/openrave/build/src/libopenrave-core:/home/dvrk-lite/ws_moveit_test/src/openrave/build/src/libopenrave:"
           NEW_RPATH "/usr/local/lib:/usr/local/lib/openrave0.9-plugins:/usr/lib/lib/x86_64-linux-gnu")
      if(CMAKE_INSTALL_DO_STRIP)
        execute_process(COMMAND "/usr/bin/strip" "${file}")
      endif()
    endif()
  endforeach()
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "openrave0.9-dp-cbindings-dev")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/openrave-0.9" TYPE FILE FILES "/home/dvrk-lite/ws_moveit_test/src/openrave/src/libopenrave-core/openrave-core_c.h")
endif()

