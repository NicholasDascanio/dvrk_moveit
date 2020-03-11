search_mode=OPTIMIZE_MAX_JOINT
srdf_filename=master.srdf
robot_name_in_srdf=master
moveit_config_pkg=dvrk_gripper2_version2
robot_name=psm_modified
planning_group_name=psm1_arm
ikfast_plugin_pkg=psm_modified_ikfast_psm1_arm_plugin
base_link_name=psm_base_link
eef_link_name=psm_tool_gripper2_link
ikfast_output_path=/home/dvrk-lite/ws_moveit_test/src/psm_modified_ikfast_psm1_arm_plugin/src/psm_modified_psm1_arm_ikfast_solver.cpp

rosrun moveit_kinematics create_ikfast_moveit_plugin.py\
  --search_mode=$search_mode\
  --srdf_filename=$srdf_filename\
  --robot_name_in_srdf=$robot_name_in_srdf\
  --moveit_config_pkg=$moveit_config_pkg\
  $robot_name\
  $planning_group_name\
  $ikfast_plugin_pkg\
  $base_link_name\
  $eef_link_name\
  $ikfast_output_path
