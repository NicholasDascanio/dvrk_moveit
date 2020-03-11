/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

#ifndef _ros_control_crtk_ros_hardware_interface_h
#define _ros_control_crtk_ros_hardware_interface_h

#include <hardware_interface/joint_state_interface.h>
#include <hardware_interface/joint_command_interface.h>
#include <hardware_interface/robot_hw.h>
#include <controller_manager/controller_manager.h>

#include <ros/ros.h>
#include <sensor_msgs/JointState.h>

namespace ros_control_crtk {

    class crtkROSHardwareInterface: public hardware_interface::RobotHW
    {

    public:

        crtkROSHardwareInterface(ros::NodeHandle & node_handle);

        virtual void read(void);

        virtual void write(void);

        bool canSwitch(const std::list<hardware_interface::ControllerInfo> & start_list,
                       const std::list<hardware_interface::ControllerInfo> & stop_list) const;

        void doSwitch(const std::list<hardware_interface::ControllerInfo> & start_list,
                      const std::list<hardware_interface::ControllerInfo> & stop_list);

    protected:
        virtual void init(void);
        void measured_js_callback(const sensor_msgs::JointState & measured_js);
        virtual void initialize_from_crtk_node(const sensor_msgs::JointState & measured_js);
        virtual void copy_measured_js_from_crtk_node(const sensor_msgs::JointState & measured_js);

        ros::NodeHandle m_node_handle;
        bool m_crtk_node_found;

        ros::Subscriber m_measured_js_subscriber;
        ros::Publisher m_servo_jp_publisher;

        hardware_interface::JointStateInterface m_joint_state_interface;
        hardware_interface::PositionJointInterface m_position_joint_interface;
        hardware_interface::VelocityJointInterface m_velocity_joint_interface;

        std::size_t m_number_of_joints;
        sensor_msgs::JointState m_measured_js; // joint state
        sensor_msgs::JointState m_servo_jp;    // commanded/servo joint position
        sensor_msgs::JointState m_servo_jv;
    };

}

#endif // _ros_control_crtk_ros_hardware_interface_h
