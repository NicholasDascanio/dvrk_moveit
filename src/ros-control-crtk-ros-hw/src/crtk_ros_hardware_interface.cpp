/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

#include <ros-control-crtk-ros-hw/crtk_ros_hardware_interface.h>

namespace ros_control_crtk {

    crtkROSHardwareInterface::crtkROSHardwareInterface(ros::NodeHandle & node_handle):
        m_node_handle(node_handle)
    {
        init();
        ROS_INFO_NAMED("crtk_hardware_interface", "Loaded crtk_hardware_interface.");
    }

    void crtkROSHardwareInterface::measured_js_callback(const sensor_msgs::JointState & measured_js)
    {
        // check if we had already found the crtk controller
        if (!m_crtk_node_found) {
            initialize_from_crtk_node(measured_js);
        }
        // preserve local copy for controllers
        copy_measured_js_from_crtk_node(measured_js);
    }

    void crtkROSHardwareInterface::initialize_from_crtk_node(const sensor_msgs::JointState & measured_js)
    {
        // let's assume everything is going to work
        m_crtk_node_found = true;

        // check that we have a valid vector of names
        if (measured_js.name.size() == 0) {
            ROS_ERROR_STREAM_THROTTLE_NAMED(60, "crtk_hardware_interface",
                                            "measured_js must have a valid vector of joint names");
            m_crtk_node_found = false;
        }
        m_number_of_joints = measured_js.name.size();
        m_measured_js.name.resize(m_number_of_joints);
        // copy names only the first time, we don't support name/size change at runtime
        std::copy(measured_js.name.begin(), measured_js.name.end(),
                  m_measured_js.name.begin());
        // resize all other vectors assuming sizes are correct
        // -- position
        {
            const size_t position_size = measured_js.position.size();
            if (position_size == m_number_of_joints) {
                m_measured_js.position.resize(m_number_of_joints);
            } else if (position_size != 0) {
                ROS_ERROR_STREAM_THROTTLE_NAMED(60, "crtk_hardware_interface",
                                                "measured_js name and position vectors must have the same size");
                m_crtk_node_found = false;
            }
        }
        // velocity
        {
            const size_t velocity_size = measured_js.velocity.size();
            if (velocity_size == m_number_of_joints) {
                m_measured_js.velocity.resize(m_number_of_joints);
            } else if (velocity_size != 0) {
                ROS_ERROR_STREAM_THROTTLE_NAMED(60, "crtk_hardware_interface",
                                                "measured_js name and velocity vectors must have the same size");
                m_crtk_node_found = false;
            }
        }
        // effort
        {
            const size_t effort_size = measured_js.effort.size();
            if (effort_size == m_number_of_joints) {
                m_measured_js.effort.resize(m_number_of_joints);
            } else if (effort_size != 0) {
                ROS_ERROR_STREAM_THROTTLE_NAMED(60, "crtk_hardware_interface",
                                                "measured_js name and effort vectors must have the same size");
                m_crtk_node_found = false;
            }
        }

        // copy joint state
        if (!m_crtk_node_found) {
            m_number_of_joints = 0;
            m_measured_js.name.resize(0);
            m_measured_js.position.resize(0);
            m_measured_js.velocity.resize(0);
            m_measured_js.effort.resize(0);
        } else {
            // register all interfaces for ros control
            for (std::size_t i = 0; i < m_number_of_joints; ++i) {
                ROS_DEBUG_STREAM_NAMED("crtk_hardware_interface",
                                       "Loading joint name: " << m_measured_js.name[i]);

                // create joint state interface
                m_joint_state_interface
                    .registerHandle(hardware_interface::JointStateHandle(m_measured_js.name[i],
                                                                         &m_measured_js.position[i],
                                                                         &m_measured_js.velocity[i],
                                                                         &m_measured_js.effort[i]));

                // create position joint interface
                m_position_joint_interface
                    .registerHandle(hardware_interface::JointHandle(m_joint_state_interface.getHandle(m_measured_js.name[i]),
                                                                    &m_measured_js.position[i]));

                // create velocity joint interface
                m_velocity_joint_interface
                    .registerHandle(hardware_interface::JointHandle(m_joint_state_interface.getHandle(m_measured_js.name[i]),
                                                                    &m_measured_js.velocity[i]));
            }
            registerInterface(&m_joint_state_interface);
            registerInterface(&m_position_joint_interface);
            registerInterface(&m_velocity_joint_interface);

            // resize servo objects too
            m_servo_jp.name.resize(m_number_of_joints);
            m_servo_jp.position.resize(m_number_of_joints);
            m_servo_jp.velocity.resize(0);
            m_servo_jp.effort.resize(0);
            m_servo_jv.name.resize(m_number_of_joints);
            m_servo_jv.position.resize(0);
            m_servo_jv.velocity.resize(m_number_of_joints);
            m_servo_jv.effort.resize(0);
            // copy names only the first time, we don't support name/size change at runtime
            std::copy(m_measured_js.name.begin(), m_measured_js.name.end(), m_servo_jp.name.begin());
            std::copy(m_measured_js.name.begin(), m_measured_js.name.end(), m_servo_jv.name.begin());

            // make the first copy from measured joint state
            copy_measured_js_from_crtk_node(measured_js);
        }
    }

    void crtkROSHardwareInterface::copy_measured_js_from_crtk_node(const sensor_msgs::JointState & measured_js)
    {
        // -- position
        {
            const size_t position_size = measured_js.position.size();
            if (position_size != 0) {
                if (position_size == m_measured_js.position.size()) {
                    std::copy(measured_js.position.begin(), measured_js.position.end(),
                              m_measured_js.position.begin());
                } else {
                    ROS_ERROR_STREAM_THROTTLE_NAMED(60, "crtk_hardware_interface",
                                                    "measured_js name and position vectors must have the same size");
                }
            }
        }
        // -- velocity
        {
            const size_t velocity_size = measured_js.velocity.size();
            if (velocity_size != 0) {
                if (velocity_size == m_measured_js.velocity.size()) {
                    std::copy(measured_js.velocity.begin(), measured_js.velocity.end(),
                              m_measured_js.velocity.begin());
                } else {
                    ROS_ERROR_STREAM_THROTTLE_NAMED(60, "crtk_hardware_interface",
                                                    "measured_js name and velocity vectors must have the same size");
                }
            }
        }
        // -- effort
        {
            const size_t effort_size = measured_js.effort.size();
            if (effort_size != 0) {
                if (effort_size == m_measured_js.effort.size()) {
                    std::copy(measured_js.effort.begin(), measured_js.effort.end(),
                              m_measured_js.effort.begin());
                } else {
                    ROS_ERROR_STREAM_THROTTLE_NAMED(60, "crtk_hardware_interface",
                                                    "measured_js name and effort vectors must have the same size");
                }
            }
        }
    }

    void crtkROSHardwareInterface::init(void)
    {
        // add subscriber to get joint state from crtk node
        m_measured_js_subscriber = m_node_handle.subscribe("measured_js", 1,
                                                           &crtkROSHardwareInterface::measured_js_callback, this);

        // add publishers to send servo commands to crtk node
        m_servo_jp_publisher = m_node_handle.advertise<sensor_msgs::JointState>("servo_jp", 1, false);
    }

    void crtkROSHardwareInterface::read(void)
    {
        // maybe we should add some code to check that we're still getting some data from the crtk node
        // estimating rate of measured_js topic might also be needed
    }

    void crtkROSHardwareInterface::write(void)
    {
        
        // if (velocity_interface_running_) {
        //     robot_->setSpeed(cmd[0], cmd[1], cmd[2], cmd[3], cmd[4], cmd[5],  max_vel_change_*125);
        // } else if (position_interface_running_) {
        //     robot_->servoj(mServoJP);
        // }
    }

    bool crtkROSHardwareInterface::canSwitch(const std::list<hardware_interface::ControllerInfo> & start_list,
                                             const std::list<hardware_interface::ControllerInfo> & stop_list) const
    {
        for (std::list<hardware_interface::ControllerInfo>::const_iterator controller_it =
                 start_list.begin(); controller_it != start_list.end();
             ++controller_it) {
            if (controller_it->name
				== "hardware_interface::VelocityJointInterface") {
                // if (velocity_interface_running_) {
                //     ROS_ERROR(
                //               "%s: An interface of that type (%s) is already running",
                //               controller_it->name.c_str(),
                //               controller_it->hardware_interface.c_str());
                //     return false;
                // }
                // if (position_interface_running_) {
                //     bool error = true;
                //     for (std::list<hardware_interface::ControllerInfo>::const_iterator stop_controller_it =
                //              stop_list.begin();
                //          stop_controller_it != stop_list.end();
                //          ++stop_controller_it) {
                //         if (stop_controller_it->name
				// 			== "hardware_interface::PositionJointInterface") {
                //             error = false;
                //             break;
                //         }
                //     }
                //     if (error) {
                //         ROS_ERROR(
                //                   "%s (type %s) can not be run simultaneously with a PositionJointInterface",
                //                   controller_it->name.c_str(),
                //                   controller_it->hardware_interface.c_str());
                //         return false;
                //     }
                // }
            } else if (controller_it->name
                       == "hardware_interface::PositionJointInterface") {
                // if (position_interface_running_) {
                //     ROS_ERROR(
                //               "%s: An interface of that type (%s) is already running",
                //               controller_it->name.c_str(),
                //               controller_it->hardware_interface.c_str());
                //     return false;
                // }
                // if (velocity_interface_running_) {
                //     bool error = true;
                //     for (std::list<hardware_interface::ControllerInfo>::const_iterator stop_controller_it =
                //              stop_list.begin();
                //          stop_controller_it != stop_list.end();
                //          ++stop_controller_it) {
                //         if (stop_controller_it->hardware_interface
				// 			== "hardware_interface::VelocityJointInterface") {
                //             error = false;
                //             break;
                //         }
                //     }
                //     if (error) {
                //         ROS_ERROR(
                //                   "%s (type %s) can not be run simultaneously with a VelocityJointInterface",
                //                   controller_it->name.c_str(),
                //                   controller_it->hardware_interface.c_str());
                //         return false;
                //     }
                // }
            }
        }

        // we can always stop a controller
        return true;
    }

    void crtkROSHardwareInterface::doSwitch(const std::list<hardware_interface::ControllerInfo> & start_list,
                                            const std::list<hardware_interface::ControllerInfo> & stop_list)
    {
        for (std::list<hardware_interface::ControllerInfo>::const_iterator controller_it =
                 stop_list.begin(); controller_it != stop_list.end();
             ++controller_it) {
            if (controller_it->name
				== "hardware_interface::VelocityJointInterface") {
                // velocity_interface_running_ = false;
                ROS_DEBUG("Stopping velocity interface");
            }
            if (controller_it->name
				== "hardware_interface::PositionJointInterface") {
                // position_interface_running_ = false;
                // std::vector<double> tmp;
                // robot_->closeServo(tmp);
                ROS_DEBUG("Stopping position interface");
            }
        }
        for (std::list<hardware_interface::ControllerInfo>::const_iterator controller_it =
                 start_list.begin(); controller_it != start_list.end();
             ++controller_it) {
            if (controller_it->name
				== "hardware_interface::VelocityJointInterface") {
                // velocity_interface_running_ = true;
                ROS_DEBUG("Starting velocity interface");
            }
            if (controller_it->name
				== "hardware_interface::PositionJointInterface") {
                // position_interface_running_ = true;
                // robot_->uploadProg();
                ROS_DEBUG("Starting position interface");
            }
        }
    }

} // namespace
