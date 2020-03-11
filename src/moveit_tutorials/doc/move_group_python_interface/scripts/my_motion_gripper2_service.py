#!/usr/bin/env python


##Devi costruire il node relativo my_motion
##DEvi scoprire da quanti valori e' l'array di joint
## e' un serial chain?

## BEGIN_SUB_TUTORIAL imports
##
## To use the Python MoveIt interfaces, we will import the `moveit_commander`_ namespace.
## This namespace provides us with a `MoveGroupCommander`_ class, a `PlanningSceneInterface`_ class,
## and a `RobotCommander`_ class. More on these below. We also import `rospy`_ and some messages that we will use:
##

import sys
import copy
import rospy
import string
import csv
import numpy
import matplotlib.pyplot as plt
import moveit_commander
import scipy
from scipy import interpolate
import sys
import sip
#import PyKDL
import moveit_msgs.msg
import geometry_msgs.msg
from math import pi
from std_msgs.msg import String
from std_msgs.msg import Header
from shape_msgs.msg import SolidPrimitive
from moveit_msgs.msg import BoundingVolume
from moveit_msgs.srv import GetCartesianPath
from moveit_msgs.srv import GetCartesianPathRequest
from moveit_msgs.srv import GetCartesianPathResponse
from moveit_msgs.srv import GetPositionFK
from moveit_msgs.srv import GetPositionFKRequest
from moveit_msgs.srv import GetPositionFKResponse
from moveit_msgs.srv import GetPositionIK
from moveit_msgs.srv import GetPositionIKRequest
from moveit_msgs.srv import GetPositionIKResponse
from moveit_msgs.msg import CollisionObject
from moveit_msgs.msg import Constraints
from moveit_msgs.msg import PositionConstraint
from sensor_msgs.msg import JointState
from geometry_msgs.msg import PoseStamped
from geometry_msgs.msg import Pose
from moveit_commander.conversions import pose_to_list
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D


## END_SUB_TUTORIAL

class GetCartPath(object):
    def __init__(self, header, group_name, link_name, avoid_collisions, path_constraints):
      rospy.loginfo("Initalizing GetCartesianPath...")
      self.group_name= group_name
      self.link_name= link_name
      self.avoid_collisions= avoid_collisions
      self.path_constraints= path_constraints
      self.header= header
      rospy.loginfo("Asking cartesian path for group: " + self.group_name)
      rospy.loginfo("End effector: " + self.link_name)
      rospy.loginfo("Setting avoid collisions to: " + str(self.avoid_collisions))
      rospy.loginfo("PoseStamped answers will be on frame: " + self.header.frame_id)
      self.cc_srv= rospy.ServiceProxy('/compute_cartesian_path', GetCartesianPath)
      rospy.loginfo("Waiting for /compute_cartesian_path service...")
      self.cc_srv.wait_for_service()
      rospy.loginfo("Connected!")
    
    def compute_cartesianPath(self, start_state, waypoints, max_step, jump_threshold):
        req = GetCartesianPathRequest()
        req.header = self.header
        req.start_state= start_state
        req.group_name= self.group_name
        req.link_name= self.link_name
        req.waypoints= waypoints
        req.max_step= max_step
        req.jump_threshold= jump_threshold
        req.avoid_collisions= self.avoid_collisions
        req.path_constraints= self.path_constraints

        try:
            resp = self.cc_srv.call(req)
            return resp
        except rospy.ServiceException as e:
            rospy.logerr("Service exception: " + str(e))
            resp = GetCartesianPathResponse()
            resp.error_code = 99999  # Failure
            return resp    
    

class GetFK(object):
    def __init__(self, fk_link, frame_id):
        """
        A class to do FK calls thru the MoveIt!'s /compute_ik service.
        :param str fk_link: link to compute the forward kinematics
        :param str frame_id: frame_id to compute the forward kinematics
        into account collisions
        """
        rospy.loginfo("Initalizing GetFK...")
        self.fk_link = fk_link
        self.frame_id = frame_id
        rospy.loginfo("Asking forward kinematics for link: " + self.fk_link)
        rospy.loginfo("PoseStamped answers will be on frame: " + self.frame_id)
        self.fk_srv = rospy.ServiceProxy('/compute_fk',
                                         GetPositionFK)
        rospy.loginfo("Waiting for /compute_fk service...")
        self.fk_srv.wait_for_service()
        rospy.loginfo("Connected!")
        self.last_js = None
        self.js_sub = rospy.Subscriber('/joint_states',
                                       JointState,
                                       self.js_cb,
                                       queue_size=1)

    def js_cb(self, data):
        self.last_js = data

    def get_current_fk_pose(self):
        resp = self.get_current_fk()
        if len(resp.pose_stamped) >= 1:
            return resp.pose_stamped[0]
        return None

    def get_current_fk(self):
        while not rospy.is_shutdown() and self.last_js is None:
            rospy.logwarn("Waiting for a /joint_states message...")
            rospy.sleep(0.1)
        return self.get_fk(self.last_js)

    def get_fk(self, joint_state, fk_link=None, frame_id=None):
        """
        Do an FK call to with.
        :param sensor_msgs/JointState joint_state: JointState message
            containing the full state of the robot.
        :param str or None fk_link: link to compute the forward kinematics for.
        """
        if fk_link is None:
            fk_link = self.fk_link

        req = GetPositionFKRequest()
        req.header.frame_id = 'world'
        req.fk_link_names = [self.fk_link]
        req.robot_state.joint_state = joint_state
        try:
            resp = self.fk_srv.call(req)
            return resp
        except rospy.ServiceException as e:
            rospy.logerr("Service exception: " + str(e))
            resp = GetPositionFKResponse()
            resp.error_code = 99999  # Failure
            return resp

class GetIK(object):
    def __init__(self, group, ik_timeout=1.0, ik_attempts=0,
                 avoid_collisions=True):
        #teoricamente il parametro delle collisioni era fissato a falso
        """
        A class to do IK calls thru the MoveIt!'s /compute_ik service.
        :param str group: MoveIt! group name
        :param float ik_timeout: default timeout for IK
        :param int ik_attempts: default number of attempts
        :param bool avoid_collisions: if to ask for IKs that take
        into account collisions
        """
        rospy.loginfo("Initalizing GetIK...")
        self.group_name = group
        self.ik_timeout = ik_timeout
        self.ik_attempts = ik_attempts
        self.avoid_collisions = avoid_collisions
        rospy.loginfo("Computing IKs for group: " + self.group_name)
        rospy.loginfo("With IK timeout: " + str(self.ik_timeout))
        rospy.loginfo("And IK attempts: " + str(self.ik_attempts))
        rospy.loginfo("Setting avoid collisions to: " +
                      str(self.avoid_collisions))
        self.ik_srv = rospy.ServiceProxy('/compute_ik',
                                         GetPositionIK)
        rospy.loginfo("Waiting for /compute_ik service...")
        self.ik_srv.wait_for_service()
        rospy.loginfo("Connected!")

    def get_ik(self, pose_stamped, rob_state,
               group=None,
               ik_timeout=None,
               ik_attempts=None,
               avoid_collisions=None):
        """
        Do an IK call to pose_stamped pose.
        :param geometry_msgs/PoseStamped pose_stamped: The 3D pose
            (with header.frame_id)
            to which compute the IK.
        :param str group: The MoveIt! group.
        :param float ik_timeout: The timeout for the IK call.
        :param int ik_attemps: The maximum # of attemps for the IK.
        :param bool avoid_collisions: If to compute collision aware IK.
        """
        if group is None:
            group = self.group_name
        if ik_timeout is None:
            ik_timeout = self.ik_timeout
        if ik_attempts is None:
            ik_attempts = self.ik_attempts
        if avoid_collisions is None:
            avoid_collisions = self.avoid_collisions


        req = GetPositionIKRequest()
        req.ik_request.robot_state = rob_state
        req.ik_request.group_name = group
        req.ik_request.pose_stamped = pose_stamped
        req.ik_request.timeout = rospy.Duration(ik_timeout)
        req.ik_request.attempts = ik_attempts
        req.ik_request.avoid_collisions = avoid_collisions
        #print(req)

        try:
            resp = self.ik_srv.call(req)
            return resp
        except rospy.ServiceException as e:
            rospy.logerr("Service exception: " + str(e))
            resp = GetPositionIKResponse()
            resp.error_code = 99999  # Failure
            return resp    
    
def all_close(goal, actual, tolerance):
  """
  Convenience method for testing if a list of values are within a tolerance of their counterparts in another list
  @param: goal       A list of floats, a Pose or a PoseStamped
  @param: actual     A list of floats, a Pose or a PoseStamped
  @param: tolerance  A float
  @returns: bool
  """
  all_equal = True
  if type(goal) is list:
    for index in range(len(goal)):
      if abs(actual[index] - goal[index]) > tolerance:
        return False

  elif type(goal) is geometry_msgs.msg.PoseStamped:
    return all_close(goal.pose, actual.pose, tolerance)

  elif type(goal) is geometry_msgs.msg.Pose:
    return all_close(pose_to_list(goal), pose_to_list(actual), tolerance)

  return True

def trajectory_conversion(joint_plan):

  via_points_array = []
  accelerations= []

  for i in range (0,len(joint_plan[1].joint_trajectory.points)):
    via_points = JointState()
    via_points.name = joint_plan[1].joint_trajectory.joint_names
    via_points.header.frame_id= 'world'
    via_points.header.seq = joint_plan[1].joint_trajectory.header.seq
    via_points.header.stamp = joint_plan[1].joint_trajectory.points[i].time_from_start
    via_points.position= joint_plan[1].joint_trajectory.points[i].positions
    via_points.velocity= joint_plan[1].joint_trajectory.points[i].velocities
    via_points.effort= joint_plan[1].joint_trajectory.points[i].effort
    via_points_array.append(via_points)
    accelerations.append(joint_plan[1].joint_trajectory.points[i].accelerations)

  gfk = GetFK('psm_tool_gripper2_link', 'world')
  cartesian_plan= []
  for j in range (0,len(via_points_array)):
    cartesian_plan.append(gfk.get_fk(via_points_array[j]))
  
  #for j in range (0,len(via_points_array)):
  #  print('Punto ' + str(j) + '\n')
  #  print(cartesian_plan[j])
  #  print('\n')    
  
  return cartesian_plan

def visualization_serialization(final_points):

  x_traj= []
  y_traj= []
  z_traj= []
    
  for i in range (0,len(final_points)):
    x_traj.append(final_points[i].pose_stamped[0].pose.position.x)
    y_traj.append(final_points[i].pose_stamped[0].pose.position.y)
    z_traj.append(final_points[i].pose_stamped[0].pose.position.z)

  return x_traj, y_traj, z_traj



class MoveGroupPythonIntefaceTutorial(object):
  """MoveGroupPythonIntefaceTutorial"""
  def __init__(self):
    super(MoveGroupPythonIntefaceTutorial, self).__init__()

    ## BEGIN_SUB_TUTORIAL setup
    ##
    ## First initialize `moveit_commander`_ and a `rospy`_ node:
    moveit_commander.roscpp_initialize(sys.argv)
    rospy.init_node('my_motion', anonymous=True)

    ## Instantiate a `RobotCommander`_ object. Provides information such as the robot's
    ## kinematic model and the robot's current joint states
    robot = moveit_commander.RobotCommander()

    ## Instantiate a `PlanningSceneInterface`_ object.  This provides a remote interface
    ## for getting, setting, and updating the robot's internal understanding of the
    ## surrounding world:
    scene = moveit_commander.PlanningSceneInterface()
    

    ## Instantiate a `MoveGroupCommander`_ object.  This object is an interface
    ## to a planning group (group of joints).  In this tutorial the group is the primary
    ## arm joints in the Panda robot, so we set the group's name to "panda_arm".
    ## If you are using a different robot, change this value to the name of your robot
    ## arm planning group.
    ## This interface can be used to plan and execute motions:
    group_name = "psm1_arm"
    move_group = moveit_commander.MoveGroupCommander(group_name)

    ## Create a `DisplayTrajectory`_ ROS publisher which is used to display
    ## trajectories in Rviz:
    display_trajectory_publisher = rospy.Publisher('/move_group/display_planned_path',
                                                   moveit_msgs.msg.DisplayTrajectory,
                                                   queue_size=20)

    ## END_SUB_TUTORIAL

    ## BEGIN_SUB_TUTORIAL basic_info
    ##
    ## Getting Basic Information
    ## ^^^^^^^^^^^^^^^^^^^^^^^^^
    # We can get the name of the reference frame for this robot:
    planning_frame = move_group.get_planning_frame()
    print("============ Planning frame: %s", planning_frame)

    # We can also print the name of the end-effector link for this group:
    eef_link = move_group.get_end_effector_link()
    print("============ End effector link 1: %s", eef_link)

    # We can get a list of all the groups in the robot:
    group_names = robot.get_group_names()
    print("============ Available Planning Groups:", robot.get_group_names())

    # Sometimes for debugging it is useful to print the entire state of the
    # robot:
    print("============ Printing robot state")
    print(robot.get_current_state())
    ## END_SUB_TUTORIAL

    # Misc variables
    self.box_name = ''
    self.sphere_name = ''
    self.cylinder_name = ''
    self.mesh_name = ''
    self.robot = robot
    self.scene = scene
    self.move_group = move_group
    self.display_trajectory_publisher = display_trajectory_publisher
    self.planning_frame = planning_frame
    self.eef_link = eef_link
    self.group_names = group_names
    self.move_group.set_num_planning_attempts(20)
    self.move_group.allow_looking(1)
    self.move_group.allow_replanning(1)
    self.move_group.set_planning_time(30)


  def go_to_joint_state(self):
    # Copy class variables to local variables to make the web tutorials more clear.
    # In practice, you should use the class variables directly unless you have a good
    # reason not to.
    move_group = self.move_group

    ## BEGIN_SUB_TUTORIAL plan_to_joint_state
    ##
    ## Planning to a Joint Goal
    ## ^^^^^^^^^^^^^^^^^^^^^^^^
    ## The Panda's zero configuration is at a `singularity <https://www.quora.com/Robotics-What-is-meant-by-kinematic-singularity>`_ so the first
    ## thing we want to do is move it to a slightly better configuration.
    # We can get the joint values from the group and adjust some of the values:
    joint_goal = move_group.get_current_joint_values()
    print('Initial Joint Values \n')
    print(joint_goal)
    for i in range(0,7):
      joint_goal[i] = 0.0
    
    print(joint_goal)
    move_group.go(joint_goal, wait=True)
    joint_goal[0] = joint_goal[0] + 0.3
    joint_goal[1] = joint_goal[1] + 0.4
    joint_goal[2] = joint_goal[2] + 0.1
    #joint_goal[5] = joint_goal[5] + 0.2
    print(joint_goal)
    # The go command can be called with joint values, poses, or without any
    # parameters if you have already set the pose or joint target for the group
    move_group.go(joint_goal, wait=True)

    # Calling ``stop()`` ensures that there is no residual movement
    move_group.stop()

    ## END_SUB_TUTORIAL

    # For testing:
    current_joints = move_group.get_current_joint_values()
    return all_close(joint_goal, current_joints, 0.01)


  def motion_interp(self):

    move_group= self.move_group
    move_group.clear_pose_targets()

    initial_pose = move_group.get_current_pose().pose

    waypoints = []
    
    #waypoints.append(copy.deepcopy(initial_pose))

    initial_pose.position.x= -0.0154421161
    initial_pose.position.y= 0.0466372503
    initial_pose.position.z= -0.072218481
    initial_pose.orientation.x= -0.5464183875
    initial_pose.orientation.y= 0.1241642006
    initial_pose.orientation.z= 0.0824836041
    initial_pose.orientation.w= 0.8241399469

    waypoints.append(copy.deepcopy(initial_pose))

    initial_pose.position.z = initial_pose.position.z - 0.06  # First move up (z)
    waypoints.append(copy.deepcopy(initial_pose))

    initial_pose.position.x = initial_pose.position.x + 0.06  # First move up (z)
    initial_pose.position.y = initial_pose.position.y + 0.06
    waypoints.append(copy.deepcopy(initial_pose))


    #x_coord= []
    #y_coord= []
    #z_coord= []
    #for i in range(0,len(waypoints)):
    #  x_coord.append(waypoints[i].position.x)
    #  y_coord.append(waypoints[i].position.y)
    #  z_coord.append(waypoints[i].position.z)

    #tck, u = scipy.interpolate.splprep([x_coord,y_coord,z_coord], k=3, s=10)
    #u_fine = numpy.linspace(0,1,300)
    #x_fine, y_fine, z_fine = scipy.interpolate.splev(u_fine, tck)


    #fig2 = plt.figure()
    #ax = Axes3D(fig2)
    #ax.set_xlabel('X Label')
    #ax.set_ylabel('Y Label')
    #ax.set_zlabel('Z Label')
    #ax.plot(x_fine, y_fine, z_fine, c='Green')
    #ax.plot(x_coord, y_coord, z_coord, linewidth=0.5, c='blue')
    #ax.scatter(x_coord, y_coord, z_coord, c='blue', marker='o')
    #plt.show()

    #adding code

    (plan, fraction) = move_group.compute_cartesian_path(
                                       waypoints,   # waypoints to follow
                                       0.005,        # eef_step, 0.005
                                       0.0)         # jump_threshold, 0.0

    print(plan)
    #plan= move_group.retime_trajectory(robot.get_current_state(), plan1)
    
    # Note: We are just planning, not asking move_group to actually move the robot yet:

    #print('Qui creo la variabile jointstate, perche la funzione per ottenere la cinematica diretta vuole in ingresso una jointstate variable')
    via_points_array = []
    accelerations= []
    for i in range (0,len(plan.joint_trajectory.points)):
        via_points = JointState()
        via_points.name = plan.joint_trajectory.joint_names
        via_points.header.frame_id= 'world'
        via_points.header.seq = plan.joint_trajectory.header.seq
        via_points.header.stamp = plan.joint_trajectory.points[i].time_from_start
        via_points.position= plan.joint_trajectory.points[i].positions
        via_points.velocity= plan.joint_trajectory.points[i].velocities
        via_points.effort= plan.joint_trajectory.points[i].effort
        via_points_array.append(via_points)
        accelerations.append(plan.joint_trajectory.points[i].accelerations)
    #print(plan)

    #writer.writerow([" ","psm_yaw_joint","psm_pitch_joint","psm_main_insertion_joint","psm_tool_roll_joint","psm_tool_pitch_joint","psm_tool_yaw_joint","psm_tool_gripper2_joint"])


    gfk = GetFK('psm_tool_gripper2_link', 'world')
    final_points= []
    for j in range (0,len(via_points_array)):
        final_points.append(gfk.get_fk(via_points_array[j]))
      
    #for j in range (0,len(via_points_array)):
        #print('Punto ' + str(j) + '\n')
        #print(final_points[j])
        #print('\n')


    #print('questi sono i waypoints da cui devo passare per avere un efficienza alta')
    #print(waypoints)

    #print('Questa e la posa iniziale da cui parte il robot, che dovrebbe essere uguale al finalpoint[0], e lo e')
    #initial_pose= move_group.get_current_pose()
    #move_group.execute(plan, wait=True)
    #print('Questa e la posa finale a cui arriva il robot, che dovrebbe essere uguale al finalpoint[ultimopunto], e lo e')
    #final_pose= move_group.get_current_pose()

    #print('Questa e la frazione di traiettoria che effettiamente seguo ( se passo da tutti i way point), ed e il 100 % perche la seguo tutta: ' + str(fraction))
    #print('Questo e il numero totale di punti della traiettoria: ' + str(len(final_points)))

    x_via_1= []
    y_via_1= []
    z_via_1= []

    waypoints.insert(0,move_group.get_current_pose().pose)
    
    for i in range (0,len(waypoints)):
      x_via_1.append(waypoints[i].position.x)
      y_via_1.append(waypoints[i].position.y)
      z_via_1.append(waypoints[i].position.z)
    
    initial_pose = move_group.get_current_pose().pose

    waypoints2 = []
    
    waypoints2.append(copy.deepcopy(initial_pose))

    initial_pose.position.x= -0.0154421161
    initial_pose.position.y= 0.0466372503
    initial_pose.position.z= -0.072218481
    initial_pose.orientation.x= -0.5464183875
    initial_pose.orientation.y= 0.1241642006
    initial_pose.orientation.z= 0.0824836041
    initial_pose.orientation.w= 0.8241399469

    waypoints2.append(copy.deepcopy(initial_pose))

    initial_pose.position.z = initial_pose.position.z - 0.03  # First move up (z)
    waypoints2.append(copy.deepcopy(initial_pose))

    initial_pose.position.z = initial_pose.position.z - 0.03  # First move up (z)
    waypoints2.append(copy.deepcopy(initial_pose))

    initial_pose.position.x = initial_pose.position.x + 0.03  # First move up (z)
    initial_pose.position.y = initial_pose.position.y + 0.03
    waypoints2.append(copy.deepcopy(initial_pose))

    initial_pose.position.x = initial_pose.position.x + 0.03  # First move up (z)
    initial_pose.position.y = initial_pose.position.y + 0.03
    waypoints2.append(copy.deepcopy(initial_pose))

    x_traj= []
    y_traj= []
    z_traj= []
    
    for i in range (0,len(final_points)):
      x_traj.append(final_points[i].pose_stamped[0].pose.position.x)
      y_traj.append(final_points[i].pose_stamped[0].pose.position.y)
      z_traj.append(final_points[i].pose_stamped[0].pose.position.z)


    x_via= []
    y_via= []
    z_via= []


    for i in range (0,len(waypoints2)):
      x_via.append(waypoints2[i].position.x)
      y_via.append(waypoints2[i].position.y)
      z_via.append(waypoints2[i].position.z)
    

    tck, u = scipy.interpolate.splprep([x_traj,y_traj,z_traj], k=5, s=5)
    u_fine = numpy.linspace(0,1,100)
    x_fine, y_fine, z_fine = scipy.interpolate.splev(u_fine, tck)

    tck1, u1 = scipy.interpolate.splprep([x_via,y_via,z_via], k=5, s=10)
    x_fine1, y_fine1, z_fine1 = scipy.interpolate.splev(u_fine, tck1)

    tck2, u2 = scipy.interpolate.splprep([x_via_1,y_via_1,z_via_1], k=3, s=10)
    x_fine2, y_fine2, z_fine2 = scipy.interpolate.splev(u_fine, tck2)

    fig = plt.figure()
    ax = Axes3D(fig)
    ax.set_xlabel('X Label')
    ax.set_ylabel('Y Label')
    ax.set_zlabel('Z Label')
    ax.plot(x_fine, y_fine, z_fine, c='Green', label="Quintic interpolation using 35 waypoints")
    ax.plot(x_fine1, y_fine1, z_fine1, c='red', label="Quintic interpolation using 6 waypoints")
    ax.plot(x_fine2, y_fine2, z_fine2, c='blue', label="Cubic interpolation using 4 waypoints")
    ax.scatter(x_traj, y_traj, z_traj, c='Green', marker='o')
    ax.scatter(x_via, y_via, z_via, c='Red', marker='o')
    ax.scatter(x_via_1, y_via_1, z_via_1, c='Blue', marker='o')
    ax.legend()
    plt.show()
    

    traj= []

    #obj= PyKDL.Frame()

  def motion_interp_with_obstacles(self, scale=1,box_pose=[], sphere_pose=[]):
    move_group= self.move_group
    move_group.clear_pose_targets()
    robot= self.robot
    
    pos_const= PositionConstraint()
    pos_const.header.frame_id = 'world'
    pos_const.link_name = 'psm_tool_gripper2_link'
    pos_const.target_point_offset= move_group.get_current_pose().pose.position
    pos_const.weight= 0.99

    shape= SolidPrimitive()
    shape.type=2
    shape.dimensions= [0.3]
    shape_pose= Pose()
    shape_pose.position= move_group.get_current_pose().pose.position
    shape_pose.orientation= move_group.get_current_pose().pose.orientation
    pos_const.constraint_region.primitives.append(shape)
    pos_const.constraint_region.primitive_poses.append(shape_pose)
    
    const= Constraints()
    const.position_constraints.append(pos_const)
    move_group.set_path_constraints(const)

    final_pose = move_group.get_current_pose().pose

    initial_pose= copy.deepcopy(final_pose)
    
    waypoints = []
    
    waypoints.append(copy.deepcopy(initial_pose))

    final_pose.position.x= -0.0154421161
    final_pose.position.y= 0.0466372503
    final_pose.position.z= -0.072218481
    final_pose.orientation.x= -0.5464183875
    final_pose.orientation.y= 0.1241642006
    final_pose.orientation.z= 0.0824836041
    final_pose.orientation.w= 0.8241399469

    waypoints.append(copy.deepcopy(final_pose))
    plan_joint,final_points, success=  self.control_trajectory(initial_pose, final_pose)
    plan_joint = move_group.go(wait=True)
    move_group.stop()
    move_group.clear_pose_targets()
    x_traj, y_traj, z_traj= visualization_serialization(final_points)
    #x_traj.append(final_pose.position.x)
    #y_traj.append(final_pose.position.y)
    #z_traj.append(final_pose.position.z)

    tck1, u = scipy.interpolate.splprep([x_traj,y_traj,z_traj], k=3, s=30)
    u_fine = numpy.linspace(0,1,100)
    x_fine1, y_fine1, z_fine1 = scipy.interpolate.splev(u_fine, tck1)
    x_1= []
    y_1= []
    z_1= []
    for i in range(0,100):
      x_1.append(x_fine1[i])
      y_1.append(y_fine1[i])
      z_1.append(z_fine1[i])


    initial_pose= copy.deepcopy(final_pose)
    final_pose.position.z = final_pose.position.z - 0.06  # First move down (z)
    waypoints.append(copy.deepcopy(final_pose))
    const.position_constraints[0].constraint_region.primitive_poses[0].position= final_pose.position
    const.position_constraints[0].constraint_region.primitive_poses[0].orientation= final_pose.orientation
    const.position_constraints[0].target_point_offset= final_pose.position
    move_group.set_path_constraints(const)

    plan_joint,final_points, success=  self.control_trajectory(initial_pose, final_pose)
    plan_joint = move_group.go(wait=True)
    move_group.stop()
    move_group.clear_pose_targets()
    x_traj1, y_traj1, z_traj1= visualization_serialization(final_points)
    x_traj1.pop(0)
    y_traj1.pop(0)
    z_traj1.pop(0)
    #x_traj1.append(final_pose.position.x)
    #y_traj1.append(final_pose.position.y)
    #z_traj1.append(final_pose.position.z)

    tck, u = scipy.interpolate.splprep([x_traj1,y_traj1,z_traj1], k=5, s=30)
    u_fine = numpy.linspace(0,1,100)
    x_fine2, y_fine2, z_fine2 = scipy.interpolate.splev(u_fine, tck)
    x_2= []
    y_2= []
    z_2= []
    for i in range(0,100):
      x_2.append(x_fine2[i])
      y_2.append(y_fine2[i])
      z_2.append(z_fine2[i])


    initial_pose= copy.deepcopy(final_pose)
    final_pose.position.x = final_pose.position.x + 0.06
    final_pose.position.y = final_pose.position.y + 0.06
    const.position_constraints[0].constraint_region.primitive_poses[0].position= final_pose.position
    const.position_constraints[0].constraint_region.primitive_poses[0].orientation= final_pose.orientation
    const.position_constraints[0].target_point_offset= final_pose.position
    move_group.set_path_constraints(const)

    waypoints.append(copy.deepcopy(final_pose))
    plan_joint,final_points, success=  self.control_trajectory(initial_pose, final_pose)  
    plan_joint = move_group.go(wait=True)
    move_group.stop()
    move_group.clear_pose_targets()
    x_traj2, y_traj2, z_traj2= visualization_serialization(final_points)
    x_traj2.pop(0)
    y_traj2.pop(0)
    z_traj2.pop(0)
    #x_traj2.append(final_pose.position.x)
    #y_traj2.append(final_pose.position.y)
    #z_traj2.append(final_pose.position.z)

    tck2, u = scipy.interpolate.splprep([x_traj2,y_traj2,z_traj2], k=5, s=30)
    u_fine = numpy.linspace(0,1,100)
    x_fine3, y_fine3, z_fine3 = scipy.interpolate.splev(u_fine, tck2)
    x_3= []
    y_3= []
    z_3= []
    for i in range(0,100):
      x_3.append(x_fine3[i])
      y_3.append(y_fine3[i])
      z_3.append(z_fine3[i])


    x_traj_tot= x_traj+x_traj1+x_traj2
    y_traj_tot= y_traj+y_traj1+y_traj2
    z_traj_tot= z_traj+z_traj1+z_traj2
    

    tck3, u = scipy.interpolate.splprep([x_traj_tot,y_traj_tot,z_traj_tot], k=5, s=30)
    u_fine = numpy.linspace(0,1,100)
    x_finetot, y_finetot, z_finetot = scipy.interpolate.splev(u_fine, tck3)

    x_traj_parz= x_1+x_2+x_3
    y_traj_parz= y_1+y_2+y_3
    z_traj_parz= z_1+z_2+z_3

    tck4, u = scipy.interpolate.splprep([x_traj_parz,y_traj_parz,z_traj_parz], k=5, s=30)
    u_fine = numpy.linspace(0,1,200)
    x_fineparz, y_fineparz, z_fineparz = scipy.interpolate.splev(u_fine, tck4)

    x_via= []
    y_via= []
    z_via= []


    for i in range (0,len(waypoints)):
      x_via.append(waypoints[i].position.x)
      y_via.append(waypoints[i].position.y)
      z_via.append(waypoints[i].position.z)
    

    x_ob= []
    y_ob= []
    z_ob= []

    obstacles= []
    obstacles.append(box_pose)
    obstacles.append(sphere_pose)

    for i in range (0,2):
      x_ob.append(obstacles[i].pose.position.x)
      y_ob.append(obstacles[i].pose.position.y)
      z_ob.append(obstacles[i].pose.position.z)

    fig = plt.figure()
    ax = Axes3D(fig)
    ax.set_xlabel('X Label')
    ax.set_ylabel('Y Label')
    ax.set_zlabel('Z Label')
    ax.plot(x_finetot, y_finetot, z_finetot, c='Red', label="Quintic interpolation using 35 waypoints")
    #ax.plot(x_fineparz, y_fineparz, z_fineparz, c='Green', label="Quintic interpolation using 35 waypoints")
    ax.scatter(x_traj_tot, y_traj_tot, z_traj_tot, c='Blue', marker='o')
    ax.scatter(x_via, y_via, z_via, c='Green', marker='o')
    ax.plot(x_via, y_via, z_via, c='Green')
    ax.scatter(x_ob, y_ob, z_ob, c='Red', marker='o')
    plt.savefig("traj.svg", format="svg")    
    plt.show()

  def control_trajectory(self, initial_pose, final_pose):

    move_group=self.move_group
    count=1
    success=0
    while(success==0):
      move_group.clear_pose_targets()
      move_group.set_pose_target(final_pose)
      plan_joint= move_group.plan()
      final_points = trajectory_conversion(plan_joint)

      for i in range (0,len(final_points)):
        if((final_pose.position.y - initial_pose.position.y < 0) and ((final_points[i].pose_stamped[0].pose.position.y > initial_pose.position.y) or (final_points[i].pose_stamped[0].pose.position.y < final_pose.position.y ))):
          success=0
          count=count+1
          break

        if((final_pose.position.y - initial_pose.position.y > 0) and ((final_points[i].pose_stamped[0].pose.position.y < initial_pose.position.y) or (final_points[i].pose_stamped[0].pose.position.y > final_pose.position.y ))):
          success=0
          count=count+1
          break

        if((final_pose.position.x - initial_pose.position.x < 0) and ((final_points[i].pose_stamped[0].pose.position.x > initial_pose.position.x) or (final_points[i].pose_stamped[0].pose.position.x < final_pose.position.x ))):
          success=0
          count=count+1
          break

        if((final_pose.position.x - initial_pose.position.x > 0) and ((final_points[i].pose_stamped[0].pose.position.x < initial_pose.position.x) or (final_points[i].pose_stamped[0].pose.position.x > final_pose.position.x ))):
          success=0
          count=count+1
          break

        if((final_pose.position.z - initial_pose.position.z < 0) and ((final_points[i].pose_stamped[0].pose.position.z > initial_pose.position.z) or (final_points[i].pose_stamped[0].pose.position.z < final_pose.position.z ))):
          success=0
          count=count+1
          break

        if((final_pose.position.z - initial_pose.position.z > 0) and ((final_points[i].pose_stamped[0].pose.position.z < initial_pose.position.z) or (final_points[i].pose_stamped[0].pose.position.z > final_pose.position.z ))):
          success=0
          count=count+1
          break

        if(((final_pose.position.y - initial_pose.position.y)==0) and ((final_points[i].pose_stamped[0].pose.position.y > (initial_pose.position.y + 0.025)) or (final_points[i].pose_stamped[0].pose.position.y <(initial_pose.position.y - 0.025)))):
          success=0
          count=count+1
          break   

        if(((final_pose.position.x - initial_pose.position.x)==0) and ((final_points[i].pose_stamped[0].pose.position.x > (initial_pose.position.x + 0.025)) or (final_points[i].pose_stamped[0].pose.position.x <(initial_pose.position.x - 0.025)))):
          success=0
          count=count+1
          break 

        if(((final_pose.position.z - initial_pose.position.z)==0) and ((final_points[i].pose_stamped[0].pose.position.z > (initial_pose.position.z + 0.025)) or (final_points[i].pose_stamped[0].pose.position.z <(initial_pose.position.z - 0.025)))):
          success=0
          count=count+1
          break        

      if(i==len(final_points)-1):
        success=1
      if(count>=10):
        success=2
    
    print(str(count)+' attempts to find a proper motion plan') 
    return plan_joint, final_points, success

  def go_to_pose_goal(self):
    # Copy class variables to local variables to make the web tutorials more clear.
    # In practice, you should use the class variables directly unless you have a good
    # reason not to.
    move_group= self.move_group
    move_group.clear_pose_targets()
    ## BEGIN_SUB_TUTORIAL plan_to_pose
    ##
    ## Planning to a Pose Goal
    ## ^^^^^^^^^^^^^^^^^^^^^^^
    ## We can plan a motion for this group to a desired pose for the
    ## end-effector:

    #joint_goal = move_group.get_current_joint_values()
    #for i in range(0,6):
    #    joint_goal[i] = 0.0

    #move_group.go(joint_goal, wait=True)
    #move_group.stop()

    pose_goal = move_group.get_current_pose().pose


    pose_goal.position.x= pose_goal.position.x + 0.01
    pose_goal.position.y= pose_goal.position.y + 0.01
    pose_goal.position.z= pose_goal.position.z + 0.01

    #puoi usare set_position_target(self, xyz, end_effector_link=""): dato che alla fine vuoi mantenere costante l'orientamento

    #move_group.set_pose_target(pose_goal)
    move_group.set_joint_value_target(pose_goal, "psm_tool_gripper2_link")


    plan= move_group.plan()

    #cosi mi assicuro di trovare un motion plan
    while len(plan[1].joint_trajectory.points) == 0:
        plan= move_group.plan()

    #plan e' un messaggio di tipo tipo Robot trajectory

    #set_max_velocity_scaling_factor(self, value) per settare velocita massima
    #set_max_acceleration_scaling_factor(self, value) per settare accelerazione massima
    #print(plan[1].joint_trajectory.points[0]) # posizioni, velocita e accellerazioni dei 7 joint nel momento iniziale piu la variabile che indica il tempo trascorso
    #print(plan[1].joint_trajectory.points[0].positions) # posizioni dei 7 joints nel momento iniziale


    plan = move_group.go(wait=True)
    move_group.stop()

    # It is always good to clear your targets after planning with poses.
    # Note: there is no equivalent function for clear_joint_value_targets()

    move_group.clear_pose_targets()

    ## END_SUB_TUTORIAL

    # For testing:
    # Note that since this section of code will not be included in the tutorials
    # we use the class variable rather than the copied state variable
    current_pose = self.move_group.get_current_pose().pose
    return all_close(pose_goal, current_pose, 0.01)


  def plan_cartesian_path(self, scale=1):
    # Copy class variables to local variables to make the web tutorials more clear.
    # In practice, you should use the class variables directly unless you have a good
    # reason not to.
    move_group = self.move_group
    robot= self.robot
    #file=open("data_joint.csv","w")
    #writer = csv.writer(file)
   # joint_goal = move_group.get_current_joint_values()
   # joint_goal[0]= -0.330729636297506
   # joint_goal[1]= 0.666837895756643
   # joint_goal[2]= 0.182516204145368
   # joint_goal[3]= 0.245536701089223
   # joint_goal[4]= -0.327690926060333
   # joint_goal[5]= 0.255689009703287
   # joint_goal[6]= 0.255849021782248
   # move_group.set_joint_value_target(joint_goal)
   # plan_joint= move_group.plan()
    #print(plan_joint)
    #plan_joint = move_group.go(wait=True)
    #raw_input()



    ## BEGIN_SUB_TUTORIAL plan_cartesian_path
    ##
    ## Cartesian Paths
    ## ^^^^^^^^^^^^^^^
    ## You can plan a Cartesian path directly by specifying a list of waypoints
    ## for the end-effector to go through. If executing  interactively in a
    ## Python shell, set scale = 1.0.
    ##
    waypoints = []

    wpose = move_group.get_current_pose().pose
    wpose.position.z = wpose.position.z - 0.05  # First move up (z)
    wpose.position.y = wpose.position.y + 0.05  # and sideways (y)
    waypoints.append(copy.deepcopy(wpose))

    wpose.position.x = wpose.position.x + 0.05  # Second move forward/backwards in (x)
    waypoints.append(copy.deepcopy(wpose))

    wpose.position.y = wpose.position.y + 0.01
    wpose.position.x = wpose.position.x + 0.01
    wpose.position.z = wpose.position.z - 0.01 
    waypoints.append(copy.deepcopy(wpose))


    #print(waypoints)
    # We want the Cartesian path to be interpolated at a resolution of 0.5 cm
    # which is why we will specify 0.005 as the eef_step in Cartesian
    # translation.  We will disable the jump threshold by setting it to 0.0,
    # ignoring the check for infeasible jumps in joint space, which is sufficient
    # for this tutorial.

    #print(self.scene.get_objects())

    (plan, fraction) = move_group.compute_cartesian_path(
                                       waypoints,   # waypoints to follow
                                       0.005,        # eef_step, 0.005
                                       0.0)         # jump_threshold, 0.0

    #plan= move_group.retime_trajectory(robot.get_current_state(), plan1)
    
    # Note: We are just planning, not asking move_group to actually move the robot yet:

    #print('Qui creo la variabile jointstate, perche la funzione per ottenere la cinematica diretta vuole in ingresso una jointstate variable')
    via_points_array = []
    accelerations= []
    for i in range (0,len(plan.joint_trajectory.points)):
        via_points = JointState()
        via_points.name = plan.joint_trajectory.joint_names
        via_points.header.frame_id= 'world'
        via_points.header.seq = plan.joint_trajectory.header.seq
        via_points.header.stamp = plan.joint_trajectory.points[i].time_from_start
        via_points.position= plan.joint_trajectory.points[i].positions
        via_points.velocity= plan.joint_trajectory.points[i].velocities
        via_points.effort= plan.joint_trajectory.points[i].effort
        via_points_array.append(via_points)
        accelerations.append(plan.joint_trajectory.points[i].accelerations)
    #print(plan)

    #writer.writerow([" ","psm_yaw_joint","psm_pitch_joint","psm_main_insertion_joint","psm_tool_roll_joint","psm_tool_pitch_joint","psm_tool_yaw_joint","psm_tool_gripper2_joint"])


    #writer.writerow(["Positions"])
    #for i in range (0,len(plan.joint_trajectory.points)):
    #  writer.writerow([i,plan.joint_trajectory.points[i].positions[0], plan.joint_trajectory.points[i].positions[1],plan.joint_trajectory.points[i].positions[2],plan.joint_trajectory.points[i].positions[3],plan.joint_trajectory.points[i].positions[4],plan.joint_trajectory.points[i].positions[5],plan.joint_trajectory.points[i].positions[6]])
    
    #writer.writerow(["Velocities"])
    #for i in range (0,len(plan.joint_trajectory.points)):
    #  writer.writerow([i,plan.joint_trajectory.points[i].velocities[0], plan.joint_trajectory.points[i].velocities[1],plan.joint_trajectory.points[i].velocities[2],plan.joint_trajectory.points[i].velocities[3],plan.joint_trajectory.points[i].velocities[4],plan.joint_trajectory.points[i].velocities[5],plan.joint_trajectory.points[i].velocities[6]])
    
    #writer.writerow(["Accellerations"])
    #for i in range (0,len(plan.joint_trajectory.points)):
    #  writer.writerow([i,plan.joint_trajectory.points[i].accelerations[0], plan.joint_trajectory.points[i].accelerations[1],plan.joint_trajectory.points[i].accelerations[2],plan.joint_trajectory.points[i].accelerations[3],plan.joint_trajectory.points[i].accelerations[4],plan.joint_trajectory.points[i].accelerations[5],plan.joint_trajectory.points[i].accelerations[6]])
    
    #file.close()

    gfk = GetFK('psm_tool_gripper2_link', 'world')
    final_points= []
    for j in range (0,len(via_points_array)):
        final_points.append(gfk.get_fk(via_points_array[j]))
      
    #for j in range (0,len(via_points_array)):
        #print('Punto ' + str(j) + '\n')
        #print(final_points[j])
        #print('\n')


    #print('questi sono i waypoints da cui devo passare per avere un efficienza alta')
    #print(waypoints)

    #print('Questa e la posa iniziale da cui parte il robot, che dovrebbe essere uguale al finalpoint[0], e lo e')
    #initial_pose= move_group.get_current_pose()
    #move_group.execute(plan, wait=True)
    #print('Questa e la posa finale a cui arriva il robot, che dovrebbe essere uguale al finalpoint[ultimopunto], e lo e')
    #final_pose= move_group.get_current_pose()

    #print('Questa e la frazione di traiettoria che effettiamente seguo ( se passo da tutti i way point), ed e il 100 % perche la seguo tutta: ' + str(fraction))
    #print('Questo e il numero totale di punti della traiettoria: ' + str(len(final_points)))

    waypoints = []

    wpose = move_group.get_current_pose().pose
    waypoints.append(copy.deepcopy(wpose))
    wpose.position.z = wpose.position.z - 0.05  # First move up (z)
    wpose.position.y = wpose.position.y + 0.05  # and sideways (y)
    waypoints.append(copy.deepcopy(wpose))

    wpose.position.x = wpose.position.x + 0.05  # Second move forward/backwards in (x)
    waypoints.append(copy.deepcopy(wpose))

    wpose.position.y = wpose.position.y + 0.01
    wpose.position.x = wpose.position.x + 0.01
    wpose.position.z = wpose.position.z - 0.01 
    waypoints.append(copy.deepcopy(wpose))

    x_traj= []
    y_traj= []
    z_traj= []
    
    for i in range (0,len(final_points)):
      x_traj.append(final_points[i].pose_stamped[0].pose.position.x)
      y_traj.append(final_points[i].pose_stamped[0].pose.position.y)
      z_traj.append(final_points[i].pose_stamped[0].pose.position.z)


    x_via= []
    y_via= []
    z_via= []


    for i in range (0,len(waypoints)):
      x_via.append(waypoints[i].position.x)
      y_via.append(waypoints[i].position.y)
      z_via.append(waypoints[i].position.z)

    tck, u = scipy.interpolate.splprep([x_traj,y_traj,z_traj], k=5, s=5)
    u_fine = numpy.linspace(0,1,200)
    x_fine, y_fine, z_fine = scipy.interpolate.splev(u_fine, tck)

    tck1, u1 = scipy.interpolate.splprep([x_via,y_via,z_via], k=3, s=10)
    x_fine1, y_fine1, z_fine1 = scipy.interpolate.splev(u_fine, tck1)

    fig = plt.figure()
    ax = Axes3D(fig)
    ax.set_xlabel('X Label')
    ax.set_ylabel('Y Label')
    ax.set_zlabel('Z Label')
    ax.plot(x_fine, y_fine, z_fine, c='Green')
    ax.plot(x_fine1, y_fine1, z_fine1, c='red')
    ax.scatter(x_traj, y_traj, z_traj, c='Blue', marker='o')
    ax.scatter(x_via, y_via, z_via, c='Red', marker='v')
    plt.show()

    #for i in range(0,len(via_points_array)):
    #  print(via_points_array[i].position)
    #  print(via_points_array[i].velocity)
    #  print(accelerations[i])


    
    #file=open("data_cartesian.csv", "w")
    #writer = csv.writer(file)
    #writer.writerow([" ", "Position x","Position y","Position z","Orientation x","Orientation y","Orientation z","Orientation w"])
    #writer.writerow(["Start pose"])
    #writer.writerow([" ",initial_pose.pose.position.x,initial_pose.pose.position.y,initial_pose.pose.position.z,initial_pose.pose.orientation.x,initial_pose.pose.orientation.y,initial_pose.pose.orientation.z,initial_pose.pose.orientation.w])
    #writer.writerow(["Final pose"])
    #writer.writerow([" ",final_pose.pose.position.x,final_pose.pose.position.y,final_pose.pose.position.z,final_pose.pose.orientation.x,final_pose.pose.orientation.y,final_pose.pose.orientation.z,final_pose.pose.orientation.w])
    
    #writer.writerow(["Cartesian via points"])
    #for i in range(0,len(final_points)):
    #  writer.writerow([i, final_points[i].pose_stamped[0].pose.position.x, final_points[i].pose_stamped[0].pose.position.y, final_points[i].pose_stamped[0].pose.position.z, final_points[i].pose_stamped[0].pose.orientation.x, final_points[i].pose_stamped[0].pose.orientation.y, final_points[i].pose_stamped[0].pose.orientation.z, final_points[i].pose_stamped[0].pose.orientation.w])

    #writer.writerow(["Time stamps via points"])   
    #for i in range (0,len(plan.joint_trajectory.points)):
    #  writer.writerow([i,plan.joint_trajectory.points[i].time_from_start])

    #writer.writerow(["X coordinates of via points"])
    #for i in range(0,len(final_points)):
    #  writer.writerow([i, x_traj[i]])

    #writer.writerow(["Y coordinates of via points"])
    #for i in range(0,len(final_points)):
    #  writer.writerow([i, y_traj[i]])

    #writer.writerow(["Z coordinates of via points"])
    #for i in range(0,len(final_points)):
    #  writer.writerow([i, z_traj[i]])

    #file.close()

    #print(plan)

    #print(robot.get_current_state())

    prova= PoseStamped()
    prova.header.frame_id= 'world'
    prova.pose= waypoints[0]
    gik= GetIK("psm1_arm")
    joint_aim= []
    joint_aim.append(gik.get_ik(prova, robot.get_current_state()))
    prova.pose= waypoints[1]
    joint_aim.append(gik.get_ik(prova, robot.get_current_state()))
    prova.pose= waypoints[2]
    joint_aim.append(gik.get_ik(prova, robot.get_current_state()))

    #print("\nprimo\n")
    #print(joint_aim[0])
    #print("\nsecondo\n")
    #print(joint_aim[1])
    #print("\nterzo\n") 
    #print(joint_aim[2])  
    joint_goal= []
    for i in range(0,8):
      if i!=6:
        joint_goal.append(joint_aim[2].solution.joint_state.position[i])
    move_group.go( joint_goal, wait=True)
    #print(move_group.get_jacobian_matrix(move_group.get_current_joint_values()))
   
    
    #for i in range (0,len(plan.joint_trajectory.points)):
    #  print(plan.joint_trajectory.points[i].time_from_start)
    return plan, fraction

    ## END_SUB_TUTORIAL


  def my_plan(self, scale=1, box_pose=[], sphere_pose=[], cylinder_pose=[], mesh_pose=[]):
    move_group = self.move_group
    robot= self.robot
    #file=open("data_joint.csv","w")
    #writer = csv.writer(file)

    waypoints = []

    initial_pose = move_group.get_current_pose().pose

    #wpose= move_group.get_current_pose().pose
    #wpose.position.z = wpose.position.z - 0.05  # First move up (z)
    #wpose.position.y = wpose.position.y + 0.05  # and sideways (y)
    #waypoints.append(copy.deepcopy(wpose))
    #wpose.position.x = wpose.position.x + 0.05  # Second move forward/backwards in (x)
    #waypoints.append(copy.deepcopy(wpose))
    #wpose.position.y = wpose.position.y + 0.01
    #wpose.position.x = wpose.position.x + 0.01
    #wpose.position.z = wpose.position.z - 0.01 
    #waypoints.append(copy.deepcopy(wpose))

    final_pose= move_group.get_current_pose().pose
    final_pose.position.z = final_pose.position.z - 0.06
    final_pose.position.y = final_pose.position.y + 0.06
    final_pose.position.x = final_pose.position.x + 0.06

    #adding code
    count=1
    success=0
    while(success==0):

      move_group.set_pose_target(final_pose)
      plan_joint= move_group.plan()
      final_points = trajectory_conversion(plan_joint)

      for i in range (0,len(final_points)):
        if(final_pose.position.y - initial_pose.position.y < 0 and final_points[i].pose_stamped[0].pose.position.y > initial_pose.position.y ):
          success=0
          count=count+1
          break

        if(final_pose.position.y - initial_pose.position.y > 0 and final_points[i].pose_stamped[0].pose.position.y < initial_pose.position.y):
          success=0
          count=count+1
          break

        if(final_pose.position.x - initial_pose.position.x > 0 and final_points[i].pose_stamped[0].pose.position.x < initial_pose.position.x):
          success=0
          count=count+1
          break

        if(final_pose.position.x - initial_pose.position.x > 0 and final_points[i].pose_stamped[0].pose.position.x < initial_pose.position.x):
          success=0
          count=count+1
          break

        if(final_pose.position.z - initial_pose.position.z > 0 and final_points[i].pose_stamped[0].pose.position.z < initial_pose.position.z ):
          success=0
          count=count+1
          break

        if(final_pose.position.z - initial_pose.position.z > 0 and final_points[i].pose_stamped[0].pose.position.z < initial_pose.position.z):
          success=0
          count=count+1
          break                                
                                
        success=1    

    #print(count)
    #code finished 

      
    #writer.writerow([" ","psm_yaw_joint","psm_pitch_joint","psm_main_insertion_joint","psm_tool_roll_joint","psm_tool_pitch_joint","psm_tool_yaw_joint","psm_tool_gripper2_joint"])
    #writer.writerow(["Positions"])
    #for i in range (0,len(plan.joint_trajectory.points)):
    #  writer.writerow([i,plan.joint_trajectory.points[i].positions[0], plan.joint_trajectory.points[i].positions[1],plan.joint_trajectory.points[i].positions[2],plan.joint_trajectory.points[i].positions[3],plan.joint_trajectory.points[i].positions[4],plan.joint_trajectory.points[i].positions[5],plan.joint_trajectory.points[i].positions[6]])
  
    #writer.writerow(["Velocities"])
    #for i in range (0,len(plan.joint_trajectory.points)):
    #  writer.writerow([i,plan.joint_trajectory.points[i].velocities[0], plan.joint_trajectory.points[i].velocities[1],plan.joint_trajectory.points[i].velocities[2],plan.joint_trajectory.points[i].velocities[3],plan.joint_trajectory.points[i].velocities[4],plan.joint_trajectory.points[i].velocities[5],plan.joint_trajectory.points[i].velocities[6]])
    
    #writer.writerow(["Accellerations"])
    #for i in range (0,len(plan.joint_trajectory.points)):
    #  writer.writerow([i,plan.joint_trajectory.points[i].accelerations[0], plan.joint_trajectory.points[i].accelerations[1],plan.joint_trajectory.points[i].accelerations[2],plan.joint_trajectory.points[i].accelerations[3],plan.joint_trajectory.points[i].accelerations[4],plan.joint_trajectory.points[i].accelerations[5],plan.joint_trajectory.points[i].accelerations[6]])
    
    #file.close()

    x_traj, y_traj, z_traj= visualization_serialization(final_points)


    x_via= []
    y_via= []
    z_via= []

    obstacles= []
    obstacles.append(box_pose)
    obstacles.append(sphere_pose)
    obstacles.append(cylinder_pose)
    obstacles.append(mesh_pose)

    for i in range (0,4):
      x_via.append(obstacles[i].pose.position.x)
      y_via.append(obstacles[i].pose.position.y)
      z_via.append(obstacles[i].pose.position.z)

    #file=open("data_cartesian.csv", "w")
    #writer = csv.writer(file)
    #writer.writerow([" ", "Position x","Position y","Position z","Orientation x","Orientation y","Orientation z","Orientation w"])
    #writer.writerow(["Start pose"])
    #writer.writerow([" ",initial_pose.pose.position.x,initial_pose.pose.position.y,initial_pose.pose.position.z,initial_pose.pose.orientation.x,initial_pose.pose.orientation.y,initial_pose.pose.orientation.z,initial_pose.pose.orientation.w])
    #writer.writerow(["Final pose"])
    #writer.writerow([" ",final_pose.pose.position.x,final_pose.pose.position.y,final_pose.pose.position.z,final_pose.pose.orientation.x,final_pose.pose.orientation.y,final_pose.pose.orientation.z,final_pose.pose.orientation.w])
    
    #writer.writerow(["Cartesian via points"])
    #for i in range(0,len(final_points)):
    #  writer.writerow([i, final_points[i].pose_stamped[0].pose.position.x, final_points[i].pose_stamped[0].pose.position.y, final_points[i].pose_stamped[0].pose.position.z, final_points[i].pose_stamped[0].pose.orientation.x, final_points[i].pose_stamped[0].pose.orientation.y, final_points[i].pose_stamped[0].pose.orientation.z, final_points[i].pose_stamped[0].pose.orientation.w])

    #writer.writerow(["Time stamps via points"])   
    #for i in range (0,len(plan.joint_trajectory.points)):
    #  writer.writerow([i,plan.joint_trajectory.points[i].time_from_start])

    #writer.writerow(["X coordinates of via points"])
    #for i in range(0,len(final_points)):
    #  writer.writerow([i, x_traj[i]])

    #writer.writerow(["Y coordinates of via points"])
    #for i in range(0,len(final_points)):
    #  writer.writerow([i, y_traj[i]])

    #writer.writerow(["Z coordinates of via points"])
    #for i in range(0,len(final_points)):
    #  writer.writerow([i, z_traj[i]])

    #file.close()

    plan_joint = move_group.go(wait=True)

    grasp_pose= move_group.get_current_pose().pose
    grasp_pose.position.x= grasp_pose.position.x + 0.005
    grasp_pose.position.y= grasp_pose.position.y + 0.005

    move_group.set_pose_target(grasp_pose)
    plan_joint= move_group.plan()
    final_points= trajectory_conversion(plan_joint)
    plan_joint = move_group.go(wait=True)
    move_group.stop()

    x_traj_2, y_traj_2, z_traj_2= visualization_serialization(final_points)

    fig2 = plt.figure()
    ax = Axes3D(fig2)
    ax.set_xlabel('X Label')
    ax.set_ylabel('Y Label')
    ax.set_zlabel('Z Label')
    ax.scatter(x_traj, y_traj, z_traj, c='Blue', marker='o')
    ax.scatter(x_via, y_via, z_via, c='Red', marker='o')
    ax.scatter(x_traj_2, y_traj_2, z_traj_2, c='Green', marker='o')
    plt.show()

    ## END_SUB_TUTORIAL

  def my_cartesian_path(self, scale=1):
    move_group = self.move_group
    robot= self.robot

    waypoints = []

    wpose = move_group.get_current_pose().pose
    wpose.position.z = wpose.position.z - 0.05  # First move up (z)
    wpose.position.y = wpose.position.y + 0.05  # and sideways (y)
    waypoints.append(copy.deepcopy(wpose))

    wpose.position.x = wpose.position.x + 0.05  # Second move forward/backwards in (x)
    waypoints.append(copy.deepcopy(wpose))

    wpose.position.y = wpose.position.y + 0.01
    wpose.position.x = wpose.position.x + 0.01
    wpose.position.z = wpose.position.z - 0.01 
    waypoints.append(copy.deepcopy(wpose))


    start_state= robot.get_current_state()
    header= Header()
    header.frame_id= 'world'
    path_constraints= Constraints()

    getcartpath= GetCartPath(header, 'psm1_arm', 'psm_tool_gripper2_link', True, path_constraints)
    cart_traj= getcartpath.compute_cartesianPath(start_state, waypoints, 0.005, 0.0)

    via_points_array = []
    accelerations= []
    for i in range (0,len(cart_traj.solution.joint_trajectory.points)):
        via_points = JointState()
        via_points.name = cart_traj.solution.joint_trajectory.joint_names
        via_points.header.frame_id= 'world'
        via_points.header.seq = cart_traj.solution.joint_trajectory.header.seq
        via_points.header.stamp = cart_traj.solution.joint_trajectory.points[i].time_from_start
        via_points.position= cart_traj.solution.joint_trajectory.points[i].positions
        via_points.velocity= cart_traj.solution.joint_trajectory.points[i].velocities
        via_points.effort= cart_traj.solution.joint_trajectory.points[i].effort
        via_points_array.append(via_points)
        accelerations.append(cart_traj.solution.joint_trajectory.points[i].accelerations)

    gfk = GetFK('psm_tool_gripper2_link', 'world')
    final_points= []
    for j in range (0,len(via_points_array)):
        final_points.append(gfk.get_fk(via_points_array[j]))
      
    #for j in range (0,len(via_points_array)):
        #print('Punto ' + str(j) + '\n')
        #print(final_points[j])
        #print('\n')


    x_traj= []
    y_traj= []
    z_traj= []
    
    for i in range (0,len(final_points)):
      x_traj.append(final_points[i].pose_stamped[0].pose.position.x)
      y_traj.append(final_points[i].pose_stamped[0].pose.position.y)
      z_traj.append(final_points[i].pose_stamped[0].pose.position.z)


    x_via= []
    y_via= []
    z_via= []


    for i in range (0,len(waypoints)):
      x_via.append(waypoints[i].position.x)
      y_via.append(waypoints[i].position.y)
      z_via.append(waypoints[i].position.z)

    fig = plt.figure()
    ax = Axes3D(fig)
    ax.set_xlabel('X Label')
    ax.set_ylabel('Y Label')
    ax.set_zlabel('Z Label')
    ax.scatter(x_traj, y_traj, z_traj, c='Blue', marker='o')
    ax.scatter(x_via, y_via, z_via, c='Red', marker='v')
    plt.show()

  def display_trajectory(self, plan):
    # Copy class variables to local variables to make the web tutorials more clear.
    # In practice, you should use the class variables directly unless you have a good
    # reason not to.
    robot = self.robot
    display_trajectory_publisher = self.display_trajectory_publisher

    ## BEGIN_SUB_TUTORIAL display_trajectory
    ##
    ## Displaying a Trajectory
    ## ^^^^^^^^^^^^^^^^^^^^^^^
    ## You can ask RViz to visualize a plan (aka trajectory) for you. But the
    ## group.plan() method does this automatically so this is not that useful
    ## here (it just displays the same trajectory again):
    ##
    ## A `DisplayTrajectory`_ msg has two primary fields, trajectory_start and trajectory.
    ## We populate the trajectory_start with our current robot state to copy over
    ## any AttachedCollisionObjects and add our plan to the trajectory.
    display_trajectory = moveit_msgs.msg.DisplayTrajectory()
    display_trajectory.trajectory_start = robot.get_current_state()
    display_trajectory.trajectory.append(plan)
    print(display_trajectory.trajectory)
    # Publish
    display_trajectory_publisher.publish(display_trajectory)

    ## END_SUB_TUTORIAL


  def execute_plan(self, plan):
    # Copy class variables to local variables to make the web tutorials more clear.
    # In practice, you should use the class variables directly unless you have a good
    # reason not to.
    move_group = self.move_group

    ## BEGIN_SUB_TUTORIAL execute_plan
    ##
    ## Executing a Plan
    ## ^^^^^^^^^^^^^^^^
    ## Use execute if you would like the robot to follow
    ## the plan that has already been computed:
    move_group.execute(plan, wait=True)

    ## **Note:** The robot's current joint state must be within some tolerance of the
    ## first waypoint in the `RobotTrajectory`_ or ``execute()`` will fail
    ## END_SUB_TUTORIAL


  def wait_for_state_update(self, box_is_known=False, box_is_attached=False, timeout=4):
    # Copy class variables to local variables to make the web tutorials more clear.
    # In practice, you should use the class variables directly unless you have a good
    # reason not to.
    box_name = self.box_name
    scene = self.scene

    ## BEGIN_SUB_TUTORIAL wait_for_scene_update
    ##
    ## Ensuring Collision Updates Are Received
    ## ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    ## If the Python node dies before publishing a collision object update message, the message
    ## could get lost and the box will not appear. To ensure that the updates are
    ## made, we wait until we see the changes reflected in the
    ## ``get_attached_objects()`` and ``get_known_object_names()`` lists.
    ## For the purpose of this tutorial, we call this function after adding,
    ## removing, attaching or detaching an object in the planning scene. We then wait
    ## until the updates have been made or ``timeout`` seconds have passed
    start = rospy.get_time()
    seconds = rospy.get_time()
    while (seconds - start < timeout) and not rospy.is_shutdown():
      # Test if the box is in attached objects
      attached_objects = scene.get_attached_objects([box_name])
      is_attached = len(attached_objects.keys()) > 0

      # Test if the box is in the scene.
      # Note that attaching the box will remove it from known_objects
      is_known = box_name in scene.get_known_object_names()

      # Test if we are in the expected state
      if (box_is_attached == is_attached) and (box_is_known == is_known):
        return True

      # Sleep so that we give other threads time on the processor
      rospy.sleep(0.1)
      seconds = rospy.get_time()
    # If we exited the while loop without returning then we timed out
    return False
    ## END_SUB_TUTORIAL


  def add_obstacles(self, timeout=4):
    box_name = self.box_name
    sphere_name= self.sphere_name 
    scene = self.scene


    #sphere
    sphere_pose = geometry_msgs.msg.PoseStamped()
    sphere_pose.header.frame_id = "world"
    sphere_pose.pose.position.x = -0.0154421161
    sphere_pose.pose.position.y = 0.0466372503
    sphere_pose.pose.position.z = -0.102218481
    sphere_pose.pose.orientation.x = -0.546021079508
    sphere_pose.pose.orientation.y = 0.124908465751
    sphere_pose.pose.orientation.z = 0.0825810613206
    sphere_pose.pose.orientation.w = 0.824281034738
    sphere_name = "sphere"
    scene.add_sphere(sphere_name, sphere_pose, radius=0.006)

    #box
    box_pose = geometry_msgs.msg.PoseStamped()
    box_pose.header.frame_id = "world"
    box_pose.pose.position.x = 0.0154421161
    box_pose.pose.position.y = 0.0766372503
    box_pose.pose.position.z = -0.132218481
    box_pose.pose.orientation.x = -0.546021079508
    box_pose.pose.orientation.y = 0.124908465751
    box_pose.pose.orientation.z = 0.0825810613206
    box_pose.pose.orientation.w = 0.824281034738
    box_name = "box"
    scene.add_box(box_name, box_pose, size=(0.01,0.01,0.01))

    return self.wait_for_state_update(box_is_known=True, timeout=timeout), box_pose, sphere_pose



  def add_box(self, timeout=4):
    # Copy class variables to local variables to make the web tutorials more clear.
    # In practice, you should use the class variables directly unless you have a good
    # reason not to.
    box_name = self.box_name
    sphere_name= self.sphere_name
    cylinder_name= self.cylinder_name
    mesh_name= self.mesh_name
    scene = self.scene

    ## BEGIN_SUB_TUTORIAL add_box
    ##
    ## Adding Objects to the Planning Scene
    ## ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    ## First, we will create a box in the planning scene at the location of the left finger:

    #sphere
    sphere_pose = geometry_msgs.msg.PoseStamped()
    sphere_pose.header.frame_id = "world"
    sphere_pose.pose.position.x = 0.0096416189416
    sphere_pose.pose.position.y = 0.0966396092852
    sphere_pose.pose.position.z = -0.122067961574
    sphere_pose.pose.orientation.x = -0.546021079508
    sphere_pose.pose.orientation.y = 0.124908465751
    sphere_pose.pose.orientation.z = 0.0825810613206
    sphere_pose.pose.orientation.w = 0.824281034738
    sphere_name = "sphere"
    scene.add_sphere(sphere_name, sphere_pose, radius=0.006)

    #cylinder
    cylinder_pose = geometry_msgs.msg.PoseStamped()
    cylinder_pose.header.frame_id = "world"
    cylinder_pose.pose.position.x = 0.0106416189416
    cylinder_pose.pose.position.y = 0.056720900352566
    cylinder_pose.pose.position.z = -0.0802209271386496
    cylinder_pose.pose.orientation.x = -0.546021079508
    cylinder_pose.pose.orientation.y = 0.124908465751
    cylinder_pose.pose.orientation.z = 0.0825810613206
    cylinder_pose.pose.orientation.w = 0.824281034738
    cylinder_name = "cylinder"
    scene.add_cylinder(cylinder_name, cylinder_pose, height=0.01, radius=0.008)

    #box
    box_pose = geometry_msgs.msg.PoseStamped()
    box_pose.header.frame_id = "world"
    box_pose.pose.position.x = -0.0154914202183458
    box_pose.pose.position.y = 0.076720900352566
    box_pose.pose.position.z = -0.102209271386496
    box_pose.pose.orientation.x = -0.546021079508
    box_pose.pose.orientation.y = 0.124908465751
    box_pose.pose.orientation.z = 0.0825810613206
    box_pose.pose.orientation.w = 0.824281034738
    box_name = "box"
    scene.add_box(box_name, box_pose, size=(0.01,0.01,0.01))

    #mesh
    mesh_pose = geometry_msgs.msg.PoseStamped()
    mesh_pose.header.frame_id = "world"
    mesh_pose.pose.position.x = 0.060411027889
    mesh_pose.pose.position.y = 0.0654289456308
    mesh_pose.pose.position.z = -0.110651832605
    mesh_pose.pose.orientation.x = -0.546021079508
    mesh_pose.pose.orientation.y = 0.124908465751
    mesh_pose.pose.orientation.z = 0.0825810613206
    mesh_pose.pose.orientation.w = 0.824281034738
    mesh_name = "mesh"
    scene.add_mesh(mesh_name, mesh_pose, filename="main_insertion_link.STL", size=(0.7,0.7,0.7))
    #se metti il kidney metti 0.001 in tutte e tre le dimensioni

    ## END_SUB_TUTORIAL
    # Copy local variables back to class variables. In practice, you should use the class
    # variables directly unless you have a good reason not to.
    self.box_name=box_name
    return self.wait_for_state_update(box_is_known=True, timeout=timeout), box_pose, sphere_pose,  cylinder_pose, mesh_pose


  def attach_box(self, timeout=4):
    # Copy class variables to local variables to make the web tutorials more clear.
    # In practice, you should use the class variables directly unless you have a good
    # reason not to.
    box_name = self.box_name
    robot = self.robot
    scene = self.scene
    eef_link = self.eef_link
    group_names = self.group_names

    ## BEGIN_SUB_TUTORIAL attach_object
    ##
    ## Attaching Objects to the Robot
    ## ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    ## Next, we will attach the box to the Panda wrist. Manipulating objects requires the
    ## robot be able to touch them without the planning scene reporting the contact as a
    ## collision. By adding link names to the ``touch_links`` array, we are telling the
    ## planning scene to ignore collisions between those links and the box. For the Panda
    ## robot, we set ``grasping_group = 'hand'``. If you are using a different robot,
    ## you should change this value to the name of your end effector group name.
    grasping_group = 'hand'
    touch_links = robot.get_link_names(group=grasping_group)
    scene.attach_box(eef_link, box_name, touch_links=touch_links)
    ## END_SUB_TUTORIAL

    # We wait for the planning scene to update.
    return self.wait_for_state_update(box_is_attached=True, box_is_known=False, timeout=timeout)


  def detach_box(self, timeout=4):
    # Copy class variables to local variables to make the web tutorials more clear.
    # In practice, you should use the class variables directly unless you have a good
    # reason not to.
    box_name = self.box_name
    scene = self.scene
    eef_link = self.eef_link

    ## BEGIN_SUB_TUTORIAL detach_object
    ##
    ## Detaching Objects from the Robot
    ## ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    ## We can also detach and remove the object from the planning scene:
    scene.remove_attached_object(eef_link, name=box_name)
    ## END_SUB_TUTORIAL

    # We wait for the planning scene to update.
    return self.wait_for_state_update(box_is_known=True, box_is_attached=False, timeout=timeout)


  def remove_box(self, timeout=4):
    # Copy class variables to local variables to make the web tutorials more clear.
    # In practice, you should use the class variables directly unless you have a good
    # reason not to.
    box_name = self.box_name
    sphere_name= self.sphere_name
    cylinder_name= self.cylinder_name
    mesh_name= self.mesh_name    
    scene = self.scene

    ## BEGIN_SUB_TUTORIAL remove_object
    ##
    ## Removing Objects from the Planning Scene
    ## ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    ## We can remove the box from the world.
    scene.remove_world_object(box_name)
    scene.remove_world_object(sphere_name)
    scene.remove_world_object(cylinder_name)
    scene.remove_world_object(mesh_name)

    ## **Note:** The object must be detached before we can remove it from the world
    ## END_SUB_TUTORIAL

    # We wait for the planning scene to update.
    return self.wait_for_state_update(box_is_attached=False, box_is_known=False, timeout=timeout)


def main():
  try:
    print(" ")
    print("----------------------------------------------------------")
    print("Welcome to the MoveIt MoveGroup Python Interface Tutorial")
    print("----------------------------------------------------------")
    print("Press Ctrl-D to exit at any time")
    print("")
    print("============ Press `Enter` to begin the tutorial by setting up the moveit_commander ...")


    #raw_input()
    tutorial = MoveGroupPythonIntefaceTutorial()


    print("============ Press `Enter` to execute a movement using a joint state goal ...")
    raw_input()
    tutorial.go_to_joint_state()

    #print "============ Press `Enter` to execute a movement using a pose goal ..."
    #raw_input()
    #tutorial.go_to_pose_goal()

    #print "============ Press `Enter` to add some objects to the planning scene ..."
    #raw_input()
    #boole, box_pose, sphere_pose, cylinder_pose, mesh_pose = tutorial.add_box()

    print "============ Press `Enter` to add some obstacles to the planning scene ..."
    raw_input()
    boole, box_pose, sphere_pose = tutorial.add_obstacles()

    print("============ Press `Enter` to plan and display a raw motion plan with obstacles ...")
    raw_input()
    tutorial.motion_interp_with_obstacles(1, box_pose, sphere_pose)

    #print("============ Press `Enter` to plan and display a raw motion plan ...")
    #raw_input()
    #tutorial.motion_interp()

    #print "============ Press `Enter` to plan and display a Cartesian path ..."
    #raw_input()
    #cartesian_plan, fraction = tutorial.plan_cartesian_path()

    #print "============ Press `Enter` to plan and display my cartesian path ..."
    #raw_input()
    #tutorial.my_cartesian_path()  

    #print "============ Press `Enter` to plan and display my plan ..."
    #raw_input()
    #tutorial.my_plan(1, box_pose, sphere_pose, cylinder_pose, mesh_pose)

    #print "============ Press `Enter` to display a saved trajectory (this will replay the Cartesian path)  ..."
    #raw_input()
    #tutorial.display_trajectory(cartesian_plan)

    #print "============ Press `Enter` to execute a saved path ..."
    #raw_input()
    #tutorial.execute_plan(cartesian_plan)

    #print "============ Press `Enter` to add a box to the planning scene ..."
    #raw_input()
    #tutorial.add_box()

    #print "============ Press `Enter` to attach a Box to the Panda robot ..."
    #raw_input()
    #tutorial.attach_box()

    #print "============ Press `Enter` to plan and execute a path with an attached collision object ..."
    #raw_input()
    #cartesian_plan, fraction = tutorial.plan_cartesian_path(scale=-1)
    #tutorial.execute_plan(cartesian_plan)

    #print "============ Press `Enter` to detach the box from the Panda robot ..."
    #raw_input()
    #tutorial.detach_box()

    print "============ Press `Enter` to remove all the objects from the planning scene ..."
    raw_input()
    tutorial.remove_box()

    print("============ Python tutorial demo complete!")
  except rospy.ROSInterruptException:
    return
  except KeyboardInterrupt:
    return

if __name__ == '__main__':
  main()

## BEGIN_TUTORIAL
## .. _moveit_commander:
##    http://docs.ros.org/melodic/api/moveit_commander/html/namespacemoveit__commander.html
##
## .. _MoveGroupCommander:
##    http://docs.ros.org/melodic/api/moveit_commander/html/classmoveit__commander_1_1move__group_1_1MoveGroupCommander.html
##
## .. _RobotCommander:
##    http://docs.ros.org/melodic/api/moveit_commander/html/classmoveit__commander_1_1robot_1_1RobotCommander.html
##
## .. _PlanningSceneInterface:
##    http://docs.ros.org/melodic/api/moveit_commander/html/classmoveit__commander_1_1planning__scene__interface_1_1PlanningSceneInterface.html
##
## .. _DisplayTrajectory:
##    http://docs.ros.org/melodic/api/moveit_msgs/html/msg/DisplayTrajectory.html
##
## .. _RobotTrajectory:
##    http://docs.ros.org/melodic/api/moveit_msgs/html/msg/RobotTrajectory.html
##
## .. _rospy:
##    http://docs.ros.org/melodic/api/rospy/html/
## CALL_SUB_TUTORIAL imports
## CALL_SUB_TUTORIAL setup
## CALL_SUB_TUTORIAL basic_info
## CALL_SUB_TUTORIAL plan_to_joint_state
## CALL_SUB_TUTORIAL plan_to_pose
## CALL_SUB_TUTORIAL plan_cartesian_path
## CALL_SUB_TUTORIAL display_trajectory
## CALL_SUB_TUTORIAL execute_plan
## CALL_SUB_TUTORIAL add_box
## CALL_SUB_TUTORIAL wait_for_scene_update
## CALL_SUB_TUTORIAL attach_object
## CALL_SUB_TUTORIAL detach_object
## CALL_SUB_TUTORIAL remove_object
## END_TUTORIAL
