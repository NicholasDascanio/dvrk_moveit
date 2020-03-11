#!/usr/bin/env python


import sys
import copy
import rospy
import string
import csv
import numpy
import math
import matplotlib.pyplot as plt
import moveit_commander
from moveit_commander import conversions
import scipy
from scipy import interpolate
from scipy.interpolate import interp1d,splrep
import sip
import PyKDL
import moveit_msgs.msg
import geometry_msgs.msg
from math import pi
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
from moveit_msgs.msg import PositionConstraint, OrientationConstraint
from moveit_commander.conversions import pose_to_list
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import numpy as np
import time
from time import sleep
from tf import transformations
from tf2_msgs.msg import TFMessage
from moveit_msgs.msg import RobotTrajectory
from std_msgs.msg import String, Bool, Float32, Empty, Float64MultiArray, Header
from geometry_msgs.msg import Pose, PoseStamped, Vector3, Quaternion, Wrench, WrenchStamped, TwistStamped, Point, Transform
from trajectory_msgs.msg import JointTrajectoryPoint
from sensor_msgs.msg import JointState, Joy
from config import Config
from auto_transform import Transforma
from get_tf import Nodo
from get_points import Punti

class GetFK(object): #this is the class that i have implemented to exploit the getfk service of moveit
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

class GetIK(object): #this is the class that i have implemented to exploit the getik service of moveit
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
    
def all_close(goal, actual, tolerance):#not important
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

#NOT USING THIS FUNCTION ANYMORE
#def trajectory_conversion_cart(joint_plan): 

#  via_points_array = []
#  accelerations= []

#  for i in range (0,len(joint_plan.joint_trajectory.points)):    #for compute cartesian function
#    via_points = JointState()
#    via_points.name = joint_plan.joint_trajectory.joint_names
#    via_points.header.frame_id= 'world'
#    via_points.header.seq = joint_plan.joint_trajectory.header.seq
#    via_points.header.stamp = joint_plan.joint_trajectory.points[i].time_from_start
#    via_points.position= joint_plan.joint_trajectory.points[i].positions
#    via_points.velocity= joint_plan.joint_trajectory.points[i].velocities
#    via_points.effort= joint_plan.joint_trajectory.points[i].effort
#    via_points_array.append(via_points)
#    accelerations.append(joint_plan.joint_trajectory.points[i].accelerations)

#  gfk = GetFK('psm_tool_gripper2_link', 'world') #previously it was world and psm_tool_tip_link 
#  cartesian_plan= []
#  for j in range (0,len(via_points_array)):
#    cartesian_plan.append(gfk.get_fk(via_points_array[j]))
  
  #for j in range (0,len(via_points_array)):
  #  print('Punto ' + str(j) + '\n')
  #  print(cartesian_plan[j])
  #  print('\n')    
  
#  return cartesian_plan

def trajectory_conversion_joint(joint_plan): #important, the function that i use to convert the joint space trajectory into a sequence of cartesian points
  via_points_array = []
  time_stamp= []

  for i in range (0,len(joint_plan[1].joint_trajectory.points)):  #for plan function
    via_points = JointState()
    via_points.name = joint_plan[1].joint_trajectory.joint_names
    via_points.header.frame_id= 'world' #previously it was world
    via_points.header.seq = joint_plan[1].joint_trajectory.header.seq
    via_points.header.stamp = joint_plan[1].joint_trajectory.points[i].time_from_start
    via_points.position= joint_plan[1].joint_trajectory.points[i].positions
    via_points.velocity= joint_plan[1].joint_trajectory.points[i].velocities
    via_points.effort= joint_plan[1].joint_trajectory.points[i].effort
    via_points_array.append(via_points)
    time_stamp.append(joint_plan[1].joint_trajectory.points[i].time_from_start)
    

  gfk = GetFK('psm_tool_gripper2_link', 'world') #previously it was world and   psm_tool_tip_link psm_tool_gripper2_link
  cartesian_plan= []
  for j in range (0,len(via_points_array)):
    cartesian_plan.append(gfk.get_fk(via_points_array[j]))
  
  #for j in range (0,len(via_points_array)):
  #  print('Punto ' + str(j) + '\n')
  #  print(cartesian_plan[j])
  #  print('\n')    
  
  return cartesian_plan, time_stamp

def visualization_serialization(final_points): #transforming the pose stamped cartesian traj into arrays of x,y,z

  x_traj= []
  y_traj= []
  z_traj= []
    
  for i in range (0,len(final_points)):
    x_traj.append(final_points[i].pose_stamped[0].pose.position.x)
    y_traj.append(final_points[i].pose_stamped[0].pose.position.y)
    z_traj.append(final_points[i].pose_stamped[0].pose.position.z)

  return x_traj, y_traj, z_traj

def velocity_cartesian(final_points, time_stamps):# function to derive cartesian velocity, but not important


  vel_cart= []
  for i in range(1,len(final_points)):
    dx= final_points[i].pose_stamped[0].pose.position.x - final_points[i-1].pose_stamped[0].pose.position.x
    dy= final_points[i].pose_stamped[0].pose.position.y - final_points[i-1].pose_stamped[0].pose.position.y
    dz= final_points[i].pose_stamped[0].pose.position.z - final_points[i-1].pose_stamped[0].pose.position.z
    dt= (time_stamps[i].nsecs - time_stamps[i-1].nsecs)*(10**-9) + (time_stamps[i].secs - time_stamps[i-1].secs)
    vx= dx/dt
    vy= dy/dt
    vz= dz/dt

    vel_cart.append(math.sqrt(vx**2 + vy**2 + vz**2))
  
  return vel_cart


class MoveGroupPythonIntefaceTutorial(object): #class of the tutorial moveit object
  def __init__(self):
    super(MoveGroupPythonIntefaceTutorial, self).__init__()

    moveit_commander.roscpp_initialize(sys.argv)
    #rospy.init_node('my_motion', anonymous=True)
    robot = moveit_commander.RobotCommander()
    scene = moveit_commander.PlanningSceneInterface()
    group_name = "psm1_arm"
    move_group = moveit_commander.MoveGroupCommander(group_name)
    display_trajectory_publisher = rospy.Publisher('/move_group/display_planned_path',
                                                   moveit_msgs.msg.DisplayTrajectory,
                                                   queue_size=20)
    planning_frame = move_group.get_planning_frame()
    eef_link = move_group.get_end_effector_link()
    group_names = robot.get_group_names()

    self.box_name = ''
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


#NOT USING THIS FUNCTION
  #def go_to_joint_state(self):
    # Copy class variables to local variables to make the web tutorials more clear.
    # In practice, you should use the class variables directly unless you have a good
    # reason not to.
  #  move_group = self.move_group

    ## BEGIN_SUB_TUTORIAL plan_to_joint_state
    ##
    ## Planning to a Joint Goal
    ## ^^^^^^^^^^^^^^^^^^^^^^^^
    ## The Panda's zero configuration is at a `singularity <https://www.quora.com/Robotics-What-is-meant-by-kinematic-singularity>`_ so the first
    ## thing we want to do is move it to a slightly better configuration.
    # We can get the joint values from the group and adjust some of the values:
  #  joint_goal = move_group.get_current_joint_values()
  #  for i in range(0,7):
  #    joint_goal[i] = 0.0
  #  move_group.go(joint_goal, wait=True)
  #  joint_goal[0] = joint_goal[0] + 0.0 # 0.3
  #  joint_goal[1] = joint_goal[1] + 0.2 # 0.4
  #  joint_goal[2] = joint_goal[2] + 0.05 # 0.1
  #  move_group.go(joint_goal, wait=True)
  #  move_group.stop()
  #  current_joints = move_group.get_current_joint_values()
  #  return all_close(joint_goal, current_joints, 0.01)
    
#NOT USING THIS FUNCTION AT THE MOMENT
#  def motion_interp_with_obstacles(self, scale=1,box_pose=[], sphere_pose=[]):
#    move_group= self.move_group
#    move_group.clear_pose_targets()
#    robot= self.robot
#    
#    pos_const= PositionConstraint()
#    pos_const.header.frame_id = 'world'
#    pos_const.link_name = 'psm_tool_gripper2_link'
#    pos_const.target_point_offset= move_group.get_current_pose().pose.position
#    pos_const.weight= 0.99
#
#    shape= SolidPrimitive()
#    shape.type=2
#    shape.dimensions= [0.3]
#    shape_pose= Pose()
#    shape_pose.position= move_group.get_current_pose().pose.position
#    shape_pose.orientation= move_group.get_current_pose().pose.orientation
#    pos_const.constraint_region.primitives.append(shape)
#    pos_const.constraint_region.primitive_poses.append(shape_pose)
#    
#    const= Constraints()
#    const.position_constraints.append(pos_const)
#    move_group.set_path_constraints(const)###
#
#    final_pose = move_group.get_current_pose().pose#
#
#    initial_pose= copy.deepcopy(final_pose)
    
#    waypoints = []
#    
#    waypoints.append(copy.deepcopy(initial_pose))

#    final_pose.position.x= -0.0154421161
#    final_pose.position.y= 0.0466372503
#    final_pose.position.z= -0.072218481
#    final_pose.orientation.x= -0.5464183875
#    final_pose.orientation.y= 0.1241642006
#    final_pose.orientation.z= 0.0824836041
#    final_pose.orientation.w= 0.8241399469

#    waypoints.append(copy.deepcopy(final_pose))
#    plan_joint,final_points, success=  self.control_trajectory(initial_pose, final_pose)
#    plan_joint = move_group.go(wait=True)
#    move_group.stop()
#    move_group.clear_pose_targets()
#    x_traj, y_traj, z_traj= visualization_serialization(final_points)
    #x_traj.append(final_pose.position.x)
    #y_traj.append(final_pose.position.y)
    #z_traj.append(final_pose.position.z)

#    tck1, u = scipy.interpolate.splprep([x_traj,y_traj,z_traj], k=3, s=30)
#    u_fine = numpy.linspace(0,1,100)
#    x_fine1, y_fine1, z_fine1 = scipy.interpolate.splev(u_fine, tck1)
#    x_1= []
#    y_1= []
#    z_1= []
#    for i in range(0,100):
#      x_1.append(x_fine1[i])
#      y_1.append(y_fine1[i])
#      z_1.append(z_fine1[i])#


#    initial_pose= copy.deepcopy(final_pose)
#    final_pose.position.z = final_pose.position.z - 0.06  # First move down (z)
#    waypoints.append(copy.deepcopy(final_pose))
#    const.position_constraints[0].constraint_region.primitive_poses[0].position= final_pose.position
#    const.position_constraints[0].constraint_region.primitive_poses[0].orientation= final_pose.orientation
#    const.position_constraints[0].target_point_offset= final_pose.position
#    move_group.set_path_constraints(const)#

#    plan_joint,final_points, success=  self.control_trajectory(initial_pose, final_pose)
#    plan_joint = move_group.go(wait=True)
#    move_group.stop()
#    move_group.clear_pose_targets()
#    x_traj1, y_traj1, z_traj1= visualization_serialization(final_points)
#    x_traj1.pop(0)
#    y_traj1.pop(0)
#    z_traj1.pop(0)
    #x_traj1.append(final_pose.position.x)
    #y_traj1.append(final_pose.position.y)
    #z_traj1.append(final_pose.position.z)

#    tck, u = scipy.interpolate.splprep([x_traj1,y_traj1,z_traj1], k=5, s=30)
#    u_fine = numpy.linspace(0,1,200)
#    x_fine2, y_fine2, z_fine2 = scipy.interpolate.splev(u_fine, tck)
#    x_2= []
#    y_2= []
#    z_2= []
#    for i in range(0,100):
#      x_2.append(x_fine2[i])
#      y_2.append(y_fine2[i])
#      z_2.append(z_fine2[i])#


#    initial_pose= copy.deepcopy(final_pose)
#    final_pose.position.x = final_pose.position.x + 0.06
#    final_pose.position.y = final_pose.position.y + 0.06
#    const.position_constraints[0].constraint_region.primitive_poses[0].position= final_pose.position
#    const.position_constraints[0].constraint_region.primitive_poses[0].orientation= final_pose.orientation
#    const.position_constraints[0].target_point_offset= final_pose.position
#    move_group.set_path_constraints(const)

#    waypoints.append(copy.deepcopy(final_pose))
#    plan_joint,final_points, success=  self.control_trajectory(initial_pose, final_pose)  
#    plan_joint = move_group.go(wait=True)
#    move_group.stop()
#    move_group.clear_pose_targets()
#    x_traj2, y_traj2, z_traj2= visualization_serialization(final_points)
#    x_traj2.pop(0)
#    y_traj2.pop(0)
#    z_traj2.pop(0)
    #x_traj2.append(final_pose.position.x)
    #y_traj2.append(final_pose.position.y)
    #z_traj2.append(final_pose.position.z)

#    tck2, u = scipy.interpolate.splprep([x_traj2,y_traj2,z_traj2], k=5, s=30)
#    u_fine = numpy.linspace(0,1,100)
#    x_fine3, y_fine3, z_fine3 = scipy.interpolate.splev(u_fine, tck2)
#    x_3= []
#    y_3= []
#    z_3= []
#    for i in range(0,100):
#      x_3.append(x_fine3[i])
#      y_3.append(y_fine3[i])
#      z_3.append(z_fine3[i])
#

#    x_traj_tot= x_traj+x_traj1+x_traj2
#    y_traj_tot= y_traj+y_traj1+y_traj2
#    z_traj_tot= z_traj+z_traj1+z_traj2
    

#    tck3, u = scipy.interpolate.splprep([x_traj_tot,y_traj_tot,z_traj_tot], k=5, s=30)
#    u_fine = numpy.linspace(0,1,100)
#    x_finetot, y_finetot, z_finetot = scipy.interpolate.splev(u_fine, tck3)

#    x_traj_parz= x_1+x_2+x_3
#    y_traj_parz= y_1+y_2+y_3
#    z_traj_parz= z_1+z_2+z_3

#    tck4, u = scipy.interpolate.splprep([x_traj_parz,y_traj_parz,z_traj_parz], k=5, s=30)
#    u_fine = numpy.linspace(0,1,200)
#    x_fineparz, y_fineparz, z_fineparz = scipy.interpolate.splev(u_fine, tck4)

#    x_via= []
#    y_via= []
#    z_via= []


#    for i in range (0,len(waypoints)):
#      x_via.append(waypoints[i].position.x)
#      y_via.append(waypoints[i].position.y)
#      z_via.append(waypoints[i].position.z)
#    
#
#    x_ob= []
#    y_ob= []
#    z_ob= []

#    obstacles= []
#    obstacles.append(box_pose)
#    obstacles.append(sphere_pose)

#    for i in range (0,2):
#      x_ob.append(obstacles[i].pose.position.x)
#      y_ob.append(obstacles[i].pose.position.y)
#      z_ob.append(obstacles[i].pose.position.z)
#
#    fig = plt.figure()
#    ax = Axes3D(fig)
#    ax.set_xlabel('X')
#    ax.set_ylabel('Y')
#    ax.set_zlabel('Z')
#    ax.plot(x_finetot, y_finetot, z_finetot, c='Red', label="Quintic interpolation using 35 waypoints")
#    #ax.plot(x_fineparz, y_fineparz, z_fineparz, c='Green', label="Quintic interpolation using 35 waypoints")
#    ax.scatter(x_traj_tot, y_traj_tot, z_traj_tot, c='Blue', marker='o')
#    ax.scatter(x_via, y_via, z_via, c='Green', marker='o')
#    ax.plot(x_via, y_via, z_via, c='Green')
#    ax.scatter(x_ob, y_ob, z_ob, c='Red', marker='o') 
#    plt.show()

  #NOT USING THIS FUNCTION
#  def pykdl2posevec(self, pypunti=[]):
#    wayps= []
#    for i in range(0,len(pypunti)):
#        punto= Pose()
#        punto.position.x= pypunti[i].p[0]
#        punto.position.y= pypunti[i].p[1]
#        punto.position.z= pypunti[i].p[2]
#        #rot = numpy.array([[pypunti[i].M[0,0],pypunti[i].M[0,1],pypunti[i].M[0,2],pypunti[i].p[0]], [pypunti[i].M[1,0],pypunti[i].M[1,1],pypunti[i].M[1,2],pypunti[i].p[1]], [pypunti[i].M[2,0],pypunti[i].M[2,1],pypunti[i].M[2,2],pypunti[i].p[2]],[0,0,0,1]])
#        #quat= transformations.quaternion_from_matrix(rot)
#        #punto.orientation.x= quat[0]
#        #punto.orientation.y= quat[1]
#        #punto.orientation.z= quat[2]
#        #punto.orientation.w= quat[3]
#        punto.orientation= self.move_group.get_current_pose().pose.orientation
#        wayps.append(punto)
#    return wayps

  def py2posevec(self, pypunti=[]):#IMPORTANT, converting the waypoints into a series of pose objects
    wayps= []
    for i in range(0,len(pypunti)):
        punto= Pose()
        punto.position.x= pypunti[i][0]
        punto.position.y= pypunti[i][1]
        punto.position.z= pypunti[i][2]
        punto.orientation= self.move_group.get_current_pose().pose.orientation
        wayps.append(punto)
    return wayps

#NOT USING THIS FUNCTION ANYMORE
#  def control_trajectory(self, initial_pose, final_pose):##

#    move_group=self.move_group
#    count=1
#    success=0
#    while(success==0):
#      move_group.clear_pose_targets()
#      move_group.set_pose_target(final_pose)
#      plan_joint= move_group.plan()
#      final_points = trajectory_conversion_joint(plan_joint)

#      for i in range (0,len(final_points)):
#        if((final_pose.position.y - initial_pose.position.y < 0) and ((final_points[i].pose_stamped[0].pose.position.y > initial_pose.position.y) or (final_points[i].pose_stamped[0].pose.position.y < final_pose.position.y ))):
#          success=0
#          count=count+1
#          break

#        if((final_pose.position.y - initial_pose.position.y > 0) and ((final_points[i].pose_stamped[0].pose.position.y < initial_pose.position.y) or (final_points[i].pose_stamped[0].pose.position.y > final_pose.position.y ))):
#          success=0
#          count=count+1
#          break

#        if((final_pose.position.x - initial_pose.position.x < 0) and ((final_points[i].pose_stamped[0].pose.position.x > initial_pose.position.x) or (final_points[i].pose_stamped[0].pose.position.x < final_pose.position.x ))):
#          success=0
#          count=count+1
#          break

#        if((final_pose.position.x - initial_pose.position.x > 0) and ((final_points[i].pose_stamped[0].pose.position.x < initial_pose.position.x) or (final_points[i].pose_stamped[0].pose.position.x > final_pose.position.x ))):
#          success=0
#          count=count+1
#          break

#        if((final_pose.position.z - initial_pose.position.z < 0) and ((final_points[i].pose_stamped[0].pose.position.z > initial_pose.position.z) or (final_points[i].pose_stamped[0].pose.position.z < final_pose.position.z ))):
#          success=0
#          count=count+1
#          break

#        if((final_pose.position.z - initial_pose.position.z > 0) and ((final_points[i].pose_stamped[0].pose.position.z < initial_pose.position.z) or (final_points[i].pose_stamped[0].pose.position.z > final_pose.position.z ))):
#          success=0
#          count=count+1
#          break

#        if(((final_pose.position.y - initial_pose.position.y)==0) and ((final_points[i].pose_stamped[0].pose.position.y > (initial_pose.position.y + 0.025)) or (final_points[i].pose_stamped[0].pose.position.y <(initial_pose.position.y - 0.025)))):
#          success=0
#          count=count+1
#          break   

#        if(((final_pose.position.x - initial_pose.position.x)==0) and ((final_points[i].pose_stamped[0].pose.position.x > (initial_pose.position.x + 0.025)) or (final_points[i].pose_stamped[0].pose.position.x <(initial_pose.position.x - 0.025)))):
#          success=0
#          count=count+1
#          break 

#        if(((final_pose.position.z - initial_pose.position.z)==0) and ((final_points[i].pose_stamped[0].pose.position.z > (initial_pose.position.z + 0.025)) or (final_points[i].pose_stamped[0].pose.position.z <(initial_pose.position.z - 0.025)))):
#          success=0
#          count=count+1
#          break        

#      if(i==len(final_points)-1):
#        success=1
#      if(count>=6):
#        success=2
    
#    print(str(count)+' attempts to find a proper motion plan') 
#    return plan_joint, final_points, success#


#NOT USING THIS FUNCTION ANYMORE, COMPUT CARTESIAN PATH USES JUST STRAIGHT LINES AND IT DOESN T REPLAN WHEN THERE ARE OBSTACLES
#  def motion_interp_main_cart(self, pywaypoints=[]):#

#    move_group= self.move_group
#    waypoints= self.py2posevec(pywaypoints)
#    #print(waypoints)
#    move_group.set_pose_target(waypoints[0])
#    move_group.go(wait=True)
#    move_group.stop()
#    move_group.clear_pose_targets()

#    waypoints.pop(0)
#    (plan, fraction) = move_group.compute_cartesian_path(
#                                       waypoints,   # waypoints to follow
#                                       0.005,        # eef_step, 0.005
#                                       0.0)         # jump_threshold, 0.0#

#    final_points = trajectory_conversion_cart(plan)
#    x_traj, y_traj, z_traj= visualization_serialization(final_points)

#    tck, u = scipy.interpolate.splprep([x_traj,y_traj,z_traj], k=5, s=5)
#    u_fine = numpy.linspace(0,1,1000)
#    x_smooth, y_smooth, z_smooth = scipy.interpolate.splev(u_fine, tck)


 #   fig = plt.figure()
#    ax = Axes3D(fig)
#    ax.set_xlabel('X')
#    ax.set_ylabel('Y')
#    ax.set_zlabel('Z')
#    ax.plot(x_smooth, y_smooth, z_smooth, c='Green', label="Quintic interpolation")
#    ax.scatter(x_traj, y_traj, z_traj, c='Blue', marker='o')
#    ax.legend()
#    plt.show()

#    traj=[]
#    for i in range(0,len(x_smooth)):
#      point_i= [x_smooth[i],y_smooth[i],z_smooth[i]]
#      traj.append(point_i)
    
#    return traj

#IMPORTANT, IS THE FUNCTION THAT I M USING
  def motion_interp_main_joint(self, pywaypoints=[], frequency=200, constant_velocity= 3.0):

    move_group= self.move_group
    robot= self.robot

    #here i constraint the orientation of the gripper to the starting orientation
    constraint= OrientationConstraint()
    constraint.header.frame_id= 'world'
    constraint.orientation= move_group.get_current_pose().pose.orientation
    constraint.link_name = 'psm_tool_gripper2_link'
    constraint.absolute_x_axis_tolerance= 0.1
    constraint.absolute_y_axis_tolerance= 0.1
    constraint.absolute_z_axis_tolerance= 0.1
    constraint.weight= 1.0
    const= Constraints()
    const.orientation_constraints.append(constraint)
    move_group.set_path_constraints(const)



    waypoints= self.py2posevec(pywaypoints) #i transform the waypoints from the pykdl to pose message
    move_group.set_pose_target(waypoints[0])# i go to the initial waypoint, which is the starting point of the actual dvrk
    move_group.go(wait=True)
    move_group.stop()
    move_group.clear_pose_targets()



    move_group.set_pose_target(waypoints[1]) #first goal
    #move_group.set_position_target([waypoints[1].position.x, waypoints[1].position.y, waypoints[1].position.z])
    plan_joint= move_group.plan() #planning joint trajectory
    final_points, timestamp1 = trajectory_conversion_joint(plan_joint) # i convert it into cartesian trajectory
    plan_joint = move_group.go(wait=True)# the robot in the simulation moves to that point
    move_group.stop()
    move_group.clear_pose_targets()
    x_traj1, y_traj1, z_traj1= visualization_serialization(final_points) # convert the cartesian trajectory into arrays of x,y,z values
    x_traj1.append(waypoints[1].position.x)
    y_traj1.append(waypoints[1].position.y)
    z_traj1.append(waypoints[1].position.z)



    move_group.set_pose_target(waypoints[2]) #second goal
    #move_group.set_position_target([waypoints[2].position.x, waypoints[2].position.y, waypoints[2].position.z])
    plan_joint= move_group.plan()
    final_points, timestamp2 = trajectory_conversion_joint(plan_joint)  
    plan_joint = move_group.go(wait=True)
    move_group.stop()
    move_group.clear_pose_targets()
    x_traj2, y_traj2, z_traj2= visualization_serialization(final_points)
    x_traj2.append(waypoints[2].position.x)
    y_traj2.append(waypoints[2].position.y)
    z_traj2.append(waypoints[2].position.z)



    move_group.set_pose_target(waypoints[3]) #third goal
    #move_group.set_position_target([waypoints[3].position.x, waypoints[3].position.y, waypoints[3].position.z])
    plan_joint= move_group.plan()    
    final_points, timestamp3 = trajectory_conversion_joint(plan_joint)  
    plan_joint = move_group.go(wait=True)
    move_group.stop()
    move_group.clear_pose_targets()
    x_traj3, y_traj3, z_traj3= visualization_serialization(final_points)
    x_traj3.append(waypoints[3].position.x)
    y_traj3.append(waypoints[3].position.y)
    z_traj3.append(waypoints[3].position.z)


    #i put all the x y z arrays together
    x_traj_tot= x_traj1+x_traj2+x_traj3
    y_traj_tot= y_traj1+y_traj2+y_traj3
    z_traj_tot= z_traj1+z_traj2+z_traj3

    #first interpolation, i need it to find the distance convered in meterS
    tck_tot, u = scipy.interpolate.splprep([x_traj_tot,y_traj_tot,z_traj_tot], k=5, s=30)
    u_fine = numpy.linspace(0,1,300)
    x_smooth_tot, y_smooth_tot, z_smooth_tot = scipy.interpolate.splev(u_fine, tck_tot)


    x_via= []
    y_via= []
    z_via= []


    for i in range (0,len(waypoints)):
      x_via.append(waypoints[i].position.x) #creating the arrays with the waypoints coordinates ( for the plot)
      y_via.append(waypoints[i].position.y)
      z_via.append(waypoints[i].position.z)

    #tck_way, u = scipy.interpolate.splprep([x_via,y_via,z_via], k=3, s=20) # cubic interpolation of just the waypoints
    #u_fine = numpy.linspace(0,1,300)
    #x_smooth_way, y_smooth_way, z_smooth_way = scipy.interpolate.splev(u_fine, tck_way)


    traj_smooth=[] #creating the trajectory
    for i in range(0,len(x_smooth_tot)):
      point_i= [x_smooth_tot[i],y_smooth_tot[i],z_smooth_tot[i]]
      traj_smooth.append(point_i)



    #calculating the distance in meters
    distance_tot=0
    distance= 0
    for i in range(1,len(traj_smooth)):
      distance= math.sqrt((traj_smooth[i][0]- traj_smooth[i-1][0])**2 + (traj_smooth[i][1]- traj_smooth[i-1][1])**2 + (traj_smooth[i][2]- traj_smooth[i-1][2])**2)
      distance_tot= distance_tot + distance
    
    #basing on the distance and on the velocity in cm/s and frequency that I've chosen, i calculate duration and num samples

    duration= (distance_tot*100)/constant_velocity
    numsamples= int(round(frequency*duration))

    # I interpolate the points of the trajectory with the correct number of samples (the trajectory does't change, changes just the number of points, distance covered remains the very same, already validated)
    tck_tot, u = scipy.interpolate.splprep([x_traj_tot,y_traj_tot,z_traj_tot], k=5, s=30)
    u_fine = numpy.linspace(0,1,numsamples)
    x_smooth_tot, y_smooth_tot, z_smooth_tot = scipy.interpolate.splev(u_fine, tck_tot)

    #i calculate the distance between the closest samples to the way-points
    traj_smooth=[]
    dist_way1= []
    dist_way2= []
    dist_way3= []
    for i in range(0,len(x_smooth_tot)):
      point_i= [x_smooth_tot[i],y_smooth_tot[i],z_smooth_tot[i]]
      traj_smooth.append(point_i)
      dist_way1.append(math.sqrt((point_i[0]-waypoints[1].position.x)**2+(point_i[1]-waypoints[1].position.y)**2+(point_i[2]-waypoints[1].position.z)**2))
      dist_way2.append(math.sqrt((point_i[0]-waypoints[2].position.x)**2+(point_i[1]-waypoints[2].position.y)**2+(point_i[2]-waypoints[2].position.z)**2))
      dist_way3.append(math.sqrt((point_i[0]-waypoints[3].position.x)**2+(point_i[1]-waypoints[3].position.y)**2+(point_i[2]-waypoints[3].position.z)**2))

    # i need to transform the cartesian smooth trajectory into a joint trajectory in order to add time parameterization, then i have to transform the
    #obtained joint trajectory into a Robot trajectory, which is the one that takes the function "retime trajectory ".
    #then i get a real trajectory, with all velocities,accelerations and positions and time stamp in the joint space
    #then i get the cartesian points and finally the x,y,z arrays

    joint_traj, flag= self.from_smooth_to_joint(traj_smooth, waypoints)

    if flag==1: # if i succeded in converting all the points to the joint space, i can do the retiming
      robot_traj= self.get_robot_traj_after_ik(joint_traj)
      plan_joint_tot= move_group.retime_trajectory(robot.get_current_state(), robot_traj, 1.0, 1.0, "iterative_time_parameterization")
      plan_tot= (True, plan_joint_tot,  0.6, 1)
      final_points, timestamp2 = trajectory_conversion_joint(plan_tot)
      x_traj_fin, y_traj_fin, z_traj_fin= visualization_serialization(final_points)
      vel_cartesian= velocity_cartesian(final_points, timestamp2)


    print('The trajectory passes '+str(min(dist_way1))+' meters next to the first waypoint') #minimum distance between the trajectory and the 1 waypoint
    print('The trajectory passes '+str(min(dist_way2))+' meters next to the second waypoint') #minimum distance between the trajectory and the 2 waypoint
    print('The trajectory passes '+str(min(dist_way3))+' meters next to the third waypoint') #minimum distance between the trajectory and the 3 waypoint

    fig = plt.figure()
    ax = Axes3D(fig)
    ax.set_xlabel('X')
    ax.set_ylabel('Y')
    ax.set_zlabel('Z')
    ax.plot(x_smooth_tot, y_smooth_tot, z_smooth_tot, c='Green', linewidth=3)
    ax.scatter(x_traj_tot, y_traj_tot, z_traj_tot, c='Blue', marker='o')
    #ax.plot(x_traj_fin, y_traj_fin, z_traj_fin, c='Green', linewidth=3)
    ax.scatter(x_via, y_via, z_via, c='Red', marker='o')
    ax.legend()
    plt.show()

    #print(robot_traj)

    #20% to accelerate and 20% to decelerate
    #constant_velocity=constant_velocity/100 # i want this in cm/s
    #vec_vel= np.linspace(0, constant_velocity, num=int(round(numsamples*0.2)))

    #vel_prof=[]
    #for i in range(0,numsamples):
    #  if i<round(numsamples*0.2):
    #    vel_prof.append(vec_vel[i])

    #  if i>(numsamples - round(numsamples*0.2)):
    #    vel_prof.append(vec_vel[numsamples-i])

    #  if (i>=round(numsamples*0.2) and i<=(numsamples - round(numsamples*0.2))):
    #    vel_prof.append(constant_velocity)

    #vel_prof.pop(0)

    return traj_smooth, duration, numsamples, distance_tot

  def get_robot_traj_after_ik(self, traj_after_ik=[]): #function which allows me to transform the joint trajectory into a robot trajectory

    robot_traj= RobotTrajectory()

    for i in range(0,len(traj_after_ik)):

      point= JointTrajectoryPoint()
      robot_traj.multi_dof_joint_trajectory.header.frame_id='world'
      robot_traj.joint_trajectory.joint_names= traj_after_ik[i].solution.joint_state.name
      robot_traj.joint_trajectory.header= traj_after_ik[i].solution.joint_state.header
      point.positions= traj_after_ik[i].solution.joint_state.position
      point.velocities= traj_after_ik[i].solution.joint_state.velocity
      point.effort= traj_after_ik[i].solution.joint_state.effort
      robot_traj.joint_trajectory.points.append(point)
    
    return robot_traj

  def from_smooth_to_joint(self, traj_smooth=[], waypoints=[]): #function which tranforms the smooth trajectory from cartesian to joint

    robot= self.robot
    joint_traj= []
    start_time = time.time()
    for i in range(0,len(traj_smooth)):
      prova= PoseStamped()
      prova.header.frame_id= 'world'
      prova.pose.orientation= waypoints[0].orientation
      prova.pose.position.x = traj_smooth[i][0]
      prova.pose.position.y = traj_smooth[i][1]
      prova.pose.position.z = traj_smooth[i][2]
      gik= GetIK("psm1_arm")
      joint_traj.append(gik.get_ik(prova, robot.get_current_state()))
      print(i)
      flag=1
      if (time.time() - start_time) >= 40 : # do not know why, but sometimes it becomes slow and afterwords it gives me segmentation fault (it occured 2 times, not many), so i check if it takes longer than 40 s, then i skip retiming
        flag=0
        print('Retiming not succeded')
        break
    

    return joint_traj, flag
 #NOT IMPORTANT
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


#  def add_obstacles(self, timeout=4): #need to add obstacles, not used with main file now
#    box_name = self.box_name
#    sphere_name= self.sphere_name 
#    scene = self.scene


#    #sphere
#    sphere_pose = geometry_msgs.msg.PoseStamped()
#    sphere_pose.header.frame_id = "world"
#    sphere_pose.pose.position.x = -0.0154421161
#    sphere_pose.pose.position.y = 0.0466372503
#    sphere_pose.pose.position.z = -0.102218481
#    sphere_pose.pose.orientation.x = -0.546021079508
#    sphere_pose.pose.orientation.y = 0.124908465751
#    sphere_pose.pose.orientation.z = 0.0825810613206
#    sphere_pose.pose.orientation.w = 0.824281034738
#    sphere_name = "sphere"
    #scene.add_sphere(sphere_name, sphere_pose, radius=0.006)

    #box
#    box_pose = geometry_msgs.msg.PoseStamped()
#    box_pose.header.frame_id = "world"
#    box_pose.pose.position.x = 0.0154421161
#    box_pose.pose.position.y = 0.0766372503
#    box_pose.pose.position.z = -0.132218481
#    box_pose.pose.orientation.x = -0.546021079508
#    box_pose.pose.orientation.y = 0.124908465751
#    box_pose.pose.orientation.z = 0.0825810613206
#    box_pose.pose.orientation.w = 0.824281034738
#    box_name = "box"
    #scene.add_box(box_name, box_pose, size=(0.01,0.01,0.01))

    #mesh
#    mesh_pose_hei = geometry_msgs.msg.PoseStamped()
#    mesh_pose_hei.header.frame_id = "world"
#    mesh_pose_hei.pose.position.x = +0.043
#    mesh_pose_hei.pose.position.y = 0
#    mesh_pose_hei.pose.position.z = 0.15
#    mesh_pose_hei.pose.orientation.x = -2.59735301e-06
#    mesh_pose_hei.pose.orientation.y = 7.07108080e-01
#    mesh_pose_hei.pose.orientation.z = 7.07105483e-01
#    mesh_pose_hei.pose.orientation.w = -2.59734347e-06
#    mesh_pose_name_hei = "meshhei"
#    scene.add_mesh(mesh_pose_name_hei, mesh_pose_hei, filename="pitch_end_link.STL", size=(1,1,1))

    #mesh
#    mesh_pose = geometry_msgs.msg.PoseStamped()
#    mesh_pose.header.frame_id = "world"
#    mesh_pose.pose.position.x = 0.0
#    mesh_pose.pose.position.y = 0.0
#    mesh_pose.pose.position.z = 0.0
#    mesh_pose.pose.orientation.x = 0
#    mesh_pose.pose.orientation.y = 0
#    mesh_pose.pose.orientation.z = 0
#    mesh_pose.pose.orientation.w = 1
#    mesh_name = "mesh"
#    scene.add_mesh(mesh_name, mesh_pose, filename="main_insertion_link.STL", size=(1,1,1))

    #mesh2
#    mesh2_pose = geometry_msgs.msg.PoseStamped()
#    mesh2_pose.header.frame_id = "world"
#    mesh2_pose.pose.position.x = 0.0
#    mesh2_pose.pose.position.y = 0.0
#    mesh2_pose.pose.position.z = 0.4162 #0.4162 - qualcosa 
#    mesh2_pose.pose.orientation.x = 0
#    mesh2_pose.pose.orientation.y = 0
#    mesh2_pose.pose.orientation.z = 0
#    mesh2_pose.pose.orientation.w = 1
#    mesh2_name = "mesh2"
#    scene.add_mesh(mesh2_name, mesh2_pose, filename="tool_roll_link.STL", size=(1,1,1))#

    #mesh3
#    mesh3_pose = geometry_msgs.msg.PoseStamped()
#    mesh3_pose.header.frame_id = "world"
#    mesh3_pose.pose.position.x = 0.0
#    mesh3_pose.pose.position.y = 0.0
#    mesh3_pose.pose.position.z = 0.4162 #0.4162
#    mesh3_pose.pose.orientation.x = -0.5
#    mesh3_pose.pose.orientation.y = -0.5
#    mesh3_pose.pose.orientation.z = -0.50000184
#    mesh3_pose.pose.orientation.w = 0.49999816
#    mesh3_name = "mesh3"
#    scene.add_mesh(mesh3_name, mesh3_pose, filename="tool_pitch_link.STL", size=(1,1,1))
    #using this function transformations.quaternion_from_euler(1.5708,0,1.5708,'sxyz')
    #mesh4
#    mesh4_pose = geometry_msgs.msg.PoseStamped()
#    mesh4_pose.header.frame_id = "world"
#    mesh4_pose.pose.position.x = 0.0
#    mesh4_pose.pose.position.y = 0.0
#    mesh4_pose.pose.position.z = 0.4253 #0.4162 + 0.0091
#    mesh4_pose.pose.orientation.x = 0
#    mesh4_pose.pose.orientation.y = -0.70710808
#    mesh4_pose.pose.orientation.z = 0
#    mesh4_pose.pose.orientation.w = 0.70710548
#    mesh4_name = "mesh4"
#    scene.add_mesh(mesh4_name, mesh4_pose, filename="tool_yaw_link.STL", size=(1,1,1))

    #mesh5
#    mesh5_pose = geometry_msgs.msg.PoseStamped()
#    mesh5_pose.header.frame_id = "world"
#    mesh5_pose.pose.position.x = 0.0
#    mesh5_pose.pose.position.y = 0.0
#    mesh5_pose.pose.position.z = 0.4253 #0.4162 + 0.0091 
#    mesh5_pose.pose.orientation.x = 0.5
#    mesh5_pose.pose.orientation.y = 0.50000184
#    mesh5_pose.pose.orientation.z = 0.5
#    mesh5_pose.pose.orientation.w = 0.49999816
#    mesh5_name = "mesh5"
#    scene.add_mesh(mesh5_name, mesh5_pose, filename="tool_gripper1_link.STL", size=(1,1,1))
#
    #mesh6
#    mesh6_pose = geometry_msgs.msg.PoseStamped()
#    mesh6_pose.header.frame_id = "world"
#    mesh6_pose.pose.position.x = 0.0
#    mesh6_pose.pose.position.y = 0.0
#    mesh6_pose.pose.position.z = 0.4253 #0.4162 + 0.0091
#    mesh6_pose.pose.orientation.x = 0.5
#    mesh6_pose.pose.orientation.y = 0.50000184
#    mesh6_pose.pose.orientation.z = 0.5
#    mesh6_pose.pose.orientation.w = 0.49999816
#    mesh6_name = "mesh6"
#    scene.add_mesh(mesh6_name, mesh6_pose, filename="tool_gripper2_link.STL", size=(1,1,1))

    #provo ad otternere il tool yaw partendo dal tool pitch. EXAMPLE

    #ricavo il quaternion del pitch rispetto al base frame
#    Tbase_to_pitch = transformations.quaternion_matrix([-0.5, -0.5, -0.50000184, 0.49999816])

    #trasformo il euler angles transform da pitch a yaw che trovo nell'urdf
    #Tpitch_to_yaw = transformations.euler_matrix(1.5708, -1.5708, 3.1416, axes='sxyz')

    #tranform da base a yaw (in teoria)
    #Tbase_to_yaw = np.dot(Tpitch_to_yaw, Tbase_to_pitch)

    #ricavo the quaternion

    #quatbase_to_yaw= transformations.quaternion_from_matrix(Tbase_to_yaw)

    #print(quatbase_to_yaw) #this is right


    #return self.wait_for_state_update(box_is_known=True, timeout=timeout)#, box_pose, sphere_pose, mesh_pose

  def transformations_matrix(self): # it assignes all the transformation matrix between a link and another, derived from DH parameters

    self.T_to_yaw_from_pitch= transformations.euler_matrix(1.5708, -1.5708, 3.1416, axes='sxyz') #from pitch tool to yaw tool
    self.T_to_yaw_from_pitch[0,3]= 0.0091
    self.T_to_pitch_from_yaw= np.linalg.inv(self.T_to_yaw_from_pitch) #from yaw to pitch tool

    self.T_to_pitch_from_roll= transformations.euler_matrix(1.5708, -1.5708, 3.1416, axes='sxyz') #from roll tool to pitch tool
    self.T_to_roll_from_pitch= np.linalg.inv(self.T_to_pitch_from_roll) #from pitch to roll

    self.T_to_roll_from_mainins= transformations.euler_matrix(0, 0, -1.5708, axes='sxyz') #from main insertion tool to roll tool
    self.T_to_roll_from_mainins[2,3]= 0.4162
    self.T_to_mainins_from_roll= np.linalg.inv(self.T_to_roll_from_mainins) #from roll tool to main insertion tool

    self.T_to_mainins_from_pitchend= transformations.euler_matrix(1.5708, 0, 3.1416, axes='sxyz') #from pitch end  to roll tool
    self.T_to_mainins_from_pitchend[0,3]= 0.043
    self.T_to_mainins_from_pitchend[1,3]= -0.28809
    self.T_to_pitchend_from_mainins= np.linalg.inv(self.T_to_mainins_from_pitchend) #from main insertion tool to pitch end tool

    self.T_to_gripper2_from_yaw= transformations.euler_matrix(0,0,0, axes='sxyz') #from yaw tool to gripper2 tool
    self.T_to_yaw_from_gripper2= np.linalg.inv(self.T_to_gripper2_from_yaw) #from gripper2 tool to yaw tool

    self.T_to_gripper1_from_yaw= transformations.euler_matrix(0,0,0, axes='sxyz') #from yaw tool to gripper1 tool
    self.T_to_yaw_from_gripper1= np.linalg.inv(self.T_to_gripper1_from_yaw) #from gripper1 tool to yaw tool

  def add_obstacles_main_tools(self, yaw_main_pose, name_psm, timeout=4):


    #here i basically receive the pose of the eef of the other arms from main.py, which in turn receives it from dvrk topic.
    #Then i add the eff as an obstacle. Then through the transformation matrix previously defined i compute the pose of all the other links as consequence
    #basing on the eef pose wrt world frame. So in the end the psm1 and psm2 to the scene as obstacles
    scene = self.scene

    #yaw
    yaw_pose_quaternion= transformations.quaternion_from_matrix(yaw_main_pose)
    yaw_pose = geometry_msgs.msg.PoseStamped()
    yaw_pose.header.frame_id = "world"
    yaw_pose.pose.position.x = yaw_main_pose[0,3]
    yaw_pose.pose.position.y = yaw_main_pose[1,3]
    yaw_pose.pose.position.z = yaw_main_pose[2,3]
    yaw_pose.pose.orientation.x = yaw_pose_quaternion[0]
    yaw_pose.pose.orientation.y = yaw_pose_quaternion[1]
    yaw_pose.pose.orientation.z = yaw_pose_quaternion[2]
    yaw_pose.pose.orientation.w = yaw_pose_quaternion[3]
    self.yaw_name = name_psm + "yaw"
    scene.add_mesh(self.yaw_name, yaw_pose, filename="tool_yaw_link.STL", size=(1,1,1))

    #gripper1
    gripper1_main_pose= np.dot(yaw_main_pose, self.T_to_gripper1_from_yaw)
    gripper1_pose_quaternion= transformations.quaternion_from_matrix(gripper1_main_pose)
    gripper1_pose = geometry_msgs.msg.PoseStamped()
    gripper1_pose.header.frame_id = "world"
    gripper1_pose.pose.position.x = gripper1_main_pose[0,3]
    gripper1_pose.pose.position.y = gripper1_main_pose[1,3]
    gripper1_pose.pose.position.z = gripper1_main_pose[2,3]
    gripper1_pose.pose.orientation.x = gripper1_pose_quaternion[0]
    gripper1_pose.pose.orientation.y = gripper1_pose_quaternion[1]
    gripper1_pose.pose.orientation.z = gripper1_pose_quaternion[2]
    gripper1_pose.pose.orientation.w = gripper1_pose_quaternion[3]
    self.gripper1_name = name_psm + "gripper1"
    scene.add_mesh(self.gripper1_name, gripper1_pose, filename="tool_gripper1_link.STL", size=(1,1,1))

    #gripper2
    gripper2_main_pose= np.dot(yaw_main_pose, self.T_to_gripper2_from_yaw)
    gripper2_pose_quaternion= transformations.quaternion_from_matrix(gripper2_main_pose)
    gripper2_pose = geometry_msgs.msg.PoseStamped()
    gripper2_pose.header.frame_id = "world"
    gripper2_pose.pose.position.x = gripper2_main_pose[0,3]
    gripper2_pose.pose.position.y = gripper2_main_pose[1,3]
    gripper2_pose.pose.position.z = gripper2_main_pose[2,3]
    gripper2_pose.pose.orientation.x = gripper2_pose_quaternion[0]
    gripper2_pose.pose.orientation.y = gripper2_pose_quaternion[1]
    gripper2_pose.pose.orientation.z = gripper2_pose_quaternion[2]
    gripper2_pose.pose.orientation.w = gripper2_pose_quaternion[3]
    self.gripper2_name = name_psm + "gripper2"
    scene.add_mesh(self.gripper2_name, gripper2_pose, filename="tool_gripper2_link.STL", size=(1,1,1))

    #pitch
    pitch_main_pose= np.dot(yaw_main_pose, self.T_to_pitch_from_yaw) #pose of the pitch wrt fulcrum
    pitch_pose_quaternion= transformations.quaternion_from_matrix(pitch_main_pose)
    pitch_pose = geometry_msgs.msg.PoseStamped()
    pitch_pose.header.frame_id = "world"
    pitch_pose.pose.position.x = pitch_main_pose[0,3]
    pitch_pose.pose.position.y = pitch_main_pose[1,3]
    pitch_pose.pose.position.z = pitch_main_pose[2,3]
    pitch_pose.pose.orientation.x = pitch_pose_quaternion[0]
    pitch_pose.pose.orientation.y = pitch_pose_quaternion[1]
    pitch_pose.pose.orientation.z = pitch_pose_quaternion[2]
    pitch_pose.pose.orientation.w = pitch_pose_quaternion[3]
    self.pitch_name = name_psm + "pitch"
    scene.add_mesh(self.pitch_name, pitch_pose, filename="tool_pitch_link.STL", size=(1,1,1))

    #roll
    roll_main_pose= np.dot(pitch_main_pose, self.T_to_roll_from_pitch) #pose of the roll wrt fulcrum
    roll_pose_quaternion= transformations.quaternion_from_matrix(roll_main_pose)
    roll_pose = geometry_msgs.msg.PoseStamped()
    roll_pose.header.frame_id = "world"
    roll_pose.pose.position.x = roll_main_pose[0,3]
    roll_pose.pose.position.y = roll_main_pose[1,3]
    roll_pose.pose.position.z = roll_main_pose[2,3]
    roll_pose.pose.orientation.x = roll_pose_quaternion[0]
    roll_pose.pose.orientation.y = roll_pose_quaternion[1]
    roll_pose.pose.orientation.z = roll_pose_quaternion[2]
    roll_pose.pose.orientation.w = roll_pose_quaternion[3]
    self.roll_name = name_psm + "roll"
    scene.add_mesh(self.roll_name, roll_pose, filename="tool_roll_link.STL", size=(1,1,1))

    #main_insertion
    mainins_main_pose= np.dot(roll_main_pose , self.T_to_mainins_from_roll) #pose of the main_insertion wrt fulcrum
    mainins_pose_quaternion= transformations.quaternion_from_matrix(mainins_main_pose)
    mainins_pose = geometry_msgs.msg.PoseStamped()
    mainins_pose.header.frame_id = "world"
    mainins_pose.pose.position.x = mainins_main_pose[0,3]
    mainins_pose.pose.position.y = mainins_main_pose[1,3]
    mainins_pose.pose.position.z = mainins_main_pose[2,3]
    mainins_pose.pose.orientation.x = mainins_pose_quaternion[0]
    mainins_pose.pose.orientation.y = mainins_pose_quaternion[1]
    mainins_pose.pose.orientation.z = mainins_pose_quaternion[2]
    mainins_pose.pose.orientation.w = mainins_pose_quaternion[3]
    self.mainins_name = name_psm + "main_insertion"
    scene.add_mesh(self.mainins_name, mainins_pose, filename="main_insertion_link.STL", size=(1,1,1))

    #pitch end
    pitchend_main_pose= np.dot(mainins_main_pose , self.T_to_pitchend_from_mainins) #pose of the pitch end wrt fulcrum
    pitchend_pose_quaternion= transformations.quaternion_from_matrix(pitchend_main_pose)
    pitchend_pose = geometry_msgs.msg.PoseStamped()
    pitchend_pose.header.frame_id = "world"
    pitchend_pose.pose.position.x = pitchend_main_pose[0,3]
    pitchend_pose.pose.position.y = pitchend_main_pose[1,3]
    pitchend_pose.pose.position.z = pitchend_main_pose[2,3]
    pitchend_pose.pose.orientation.x = pitchend_pose_quaternion[0]
    pitchend_pose.pose.orientation.y = pitchend_pose_quaternion[1]
    pitchend_pose.pose.orientation.z = pitchend_pose_quaternion[2]
    pitchend_pose.pose.orientation.w = pitchend_pose_quaternion[3]
    self.pitchend_name = name_psm + "pitch_end"
    #scene.add_mesh(self.pitchend_name, pitchend_pose, filename="pitch_end_link.STL", size=(1,1,1))

  def remove_obstacles(self, timeout=4):

    box_name = self.box_name
    scene = self.scene

    scene.remove_world_object(box_name)
    scene.remove_world_object(self.mainins_name)
    scene.remove_world_object(self.roll_name)
    scene.remove_world_object(self.pitch_name)
    scene.remove_world_object(self.yaw_name)
    scene.remove_world_object(self.gripper1_name)
    scene.remove_world_object(self.gripper2_name)
    scene.remove_world_object(self.pitchend_name)

    return self.wait_for_state_update(box_is_attached=False, box_is_known=False, timeout=timeout)
