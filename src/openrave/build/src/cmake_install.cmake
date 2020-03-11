# Install script for directory: /home/dvrk-lite/ws_moveit_test/src/openrave/src

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
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/openrave0.9" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/openrave0.9")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/openrave0.9"
         RPATH "/usr/local/lib:/usr/local/lib/openrave0.9-plugins:/usr/lib/lib/x86_64-linux-gnu")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "/home/dvrk-lite/ws_moveit_test/src/openrave/build/src/openrave0.9")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/openrave0.9" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/openrave0.9")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/openrave0.9"
         OLD_RPATH "/usr/lib/lib/x86_64-linux-gnu:/home/dvrk-lite/ws_moveit_test/src/openrave/build/src/libopenrave-core:/home/dvrk-lite/ws_moveit_test/src/openrave/build/src/libopenrave:"
         NEW_RPATH "/usr/local/lib:/usr/local/lib/openrave0.9-plugins:/usr/lib/lib/x86_64-linux-gnu")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/openrave0.9")
    endif()
  endif()
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  
            if ("$ENV{DESTDIR}" STREQUAL "")
                execute_process(COMMAND "/usr/bin/cmake" -E create_symlink
                                /usr/local/bin/openrave0.9
                                /usr/local/bin/openrave)
            else ()
                execute_process(COMMAND "/usr/bin/cmake" -E create_symlink
                                /usr/local/bin/openrave0.9
                                $ENV{DESTDIR}//usr/local/bin/openrave)
            endif ()
        
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "openrave0.9-dp-dev")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/openrave-0.9/cppexamples" TYPE FILE FILES "/home/dvrk-lite/ws_moveit_test/src/openrave/src/cppexamples/FindOpenRAVE.cmake")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "openrave0.9-dp-data")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/openrave-0.9" TYPE DIRECTORY FILES "/home/dvrk-lite/ws_moveit_test/src/openrave/src/models" REGEX "/\\.svn$" EXCLUDE)
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "openrave0.9-dp-data")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/openrave-0.9" TYPE DIRECTORY FILES "/home/dvrk-lite/ws_moveit_test/src/openrave/src/robots" REGEX "/\\.svn$" EXCLUDE)
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "openrave0.9-dp-data")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/openrave-0.9" TYPE DIRECTORY FILES "/home/dvrk-lite/ws_moveit_test/src/openrave/src/data" REGEX "/\\.svn$" EXCLUDE)
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "openrave0.9-dp-dev")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/openrave-0.9" TYPE DIRECTORY FILES "/home/dvrk-lite/ws_moveit_test/src/openrave/src/cppexamples" FILES_MATCHING REGEX "/[^/]*\\.cpp$" REGEX "/[^/]*\\.xml$" REGEX "/[^/]*\\.h$" REGEX "/[^/]*\\.txt$" REGEX "/\\.svn$" EXCLUDE)
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/home/dvrk-lite/ws_moveit_test/src/openrave/build/src/libopenrave/cmake_install.cmake")
  include("/home/dvrk-lite/ws_moveit_test/src/openrave/build/src/libopenrave-core/cmake_install.cmake")

endif()

