cmake_minimum_required (VERSION 2.8.3)

project (ros_control_crtk_ros_hw)

## Find catkin macros and libraries
## if COMPONENTS list like find_package(catkin REQUIRED COMPONENTS xyz)
## is used, also find other catkin packages
find_package (catkin REQUIRED COMPONENTS
  hardware_interface
  controller_manager
  actionlib
  control_msgs
  geometry_msgs
  roscpp
  sensor_msgs
  std_msgs
  trajectory_msgs
  tf
)

catkin_package (
  INCLUDE_DIRS include
  CATKIN_DEPENDS hardware_interface controller_manager actionlib control_msgs geometry_msgs roscpp sensor_msgs trajectory_msgs
)

## Specify additional locations of header files
## Your package locations should be listed before other locations
# include_directories(include)
include_directories (include ${catkin_INCLUDE_DIRS})

# library for hardware interface
add_library (ros_control_crtk_ros_hw src/crtk_ros_hardware_interface.cpp)
target_link_libraries (ros_control_crtk_ros_hw
                       ${catkin_LIBRARIES})

# main executable, i.e. ROS node
#set(${PROJECT_NAME}_SOURCES
#    src/ur_ros_wrapper.cpp
#    src/ur_driver.cpp
#    src/ur_realtime_communication.cpp
#    src/ur_communication.cpp
#    src/robot_state.cpp
#    src/robot_state_RT.cpp
#    src/do_output.cpp)
#add_executable(ur_driver ${${PROJECT_NAME}_SOURCES})

## Add cmake target dependencies of the executable
## same as for the library above
#add_dependencies (ur_driver ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})

## Specify libraries to link a library or executable target against
# target_link_libraries(ur_driver
#  ur_hardware_interface
#  ${catkin_LIBRARIES}
# )

#############
## Install ##
#############

install (DIRECTORY launch/ DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}/launch)
install (DIRECTORY config/ DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}/config)

## Mark executables and/or libraries for installation
install (TARGETS ros_control_crtk_ros_hw #--------------------------- add executable here
         LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
         RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION})

## Mark cpp header files for installation
install (DIRECTORY include/${PROJECT_NAME}/
         DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION}
         FILES_MATCHING PATTERN "*.h")
