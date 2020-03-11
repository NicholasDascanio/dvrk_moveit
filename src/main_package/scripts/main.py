#!/usr/bin/env python



import sys
import rospy
import numpy
import copy
import numpy as np
import math
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
from time import sleep
import scipy
import dvrk
import PyKDL
from scipy import interpolate
from scipy.interpolate import interp1d,splprep
from tf import transformations
from tf2_msgs.msg import TFMessage
from std_msgs.msg import String, Bool, Float32, Empty, Float64MultiArray
from geometry_msgs.msg import Pose, PoseStamped, Vector3, Quaternion, Wrench, WrenchStamped, TwistStamped, Point, Transform, PointStamped
from sensor_msgs.msg import JointState, Joy

#from moveit_node import MoveGroupPythonIntefaceTutorial
from moveit_node_2 import MoveGroupPythonIntefaceTutorial
from config import Config
from auto_transform import Transforma
from get_tf import Nodo
from get_points import Punti

from dvrk.arm import *
#oppure from arm import
#I modified arm by adding set_base_frame(self, htrasf) and the relative publisher
#from dvrk.arm import *

from dvrk.psm import *
#I need psm because I call methods like open and close jaw
#from dvrk.psm import *

#da qui in poi dovrebbe essere il main


#not using this function at the moment
#def cubic_interp_5way(grasp_pos1=[], grasp_pos2=[], grasp_pos3=[]):
    #Function to find the cubic interpolation by considering the 3 original waypoints + 2 more waypoints among them
#    grasp_pos_x_aug= copy.deepcopy(grasp_pos1)
#    grasp_pos_y_aug= copy.deepcopy(grasp_pos2)
#    grasp_pos_z_aug= copy.deepcopy(grasp_pos3)

#    i=0
#    while i < 6:
#        grasp_pos_x_aug.insert(i+1,((grasp_pos_x_aug[i]+grasp_pos_x_aug[i+1])/2))
#        grasp_pos_y_aug.insert(i+1,((grasp_pos_y_aug[i]+grasp_pos_y_aug[i+1])/2))
#        grasp_pos_z_aug.insert(i+1,((grasp_pos_z_aug[i]+grasp_pos_z_aug[i+1])/2))
#        i=i+2
#    grasp_pos_x_aug.pop(1)
#    grasp_pos_y_aug.pop(1)
#    grasp_pos_z_aug.pop(1)

#    tck3, u = scipy.interpolate.splprep([grasp_pos_x_aug,grasp_pos_y_aug,grasp_pos_z_aug], k=3)
#    u_fine = numpy.linspace(0,1,100)
#    x_fine, y_fine, z_fine = scipy.interpolate.splev(u_fine, tck3)

#    return  x_fine, y_fine, z_fine
    ####

#not using this function at the moment
#def quintic_interp_Nway(vect_pos=[]):
    ####### Now I find the quintic polynomial by interpolating all the waypoints between each way point
#    x= []
#    y= []
#    z= []
#    x_points= []
#    y_points= []
#    z_points= []
#    num=15

#    x.append(numpy.linspace(vect_pos[0].p[0], vect_pos[1].p[0], num, endpoint = True, retstep = False, dtype = None))
#    x.append(numpy.linspace(vect_pos[1].p[0], vect_pos[2].p[0], num, endpoint = True, retstep = False, dtype = None))
#    x.append(numpy.linspace(vect_pos[2].p[0], vect_pos[3].p[0], num, endpoint = True, retstep = False, dtype = None))

#    y.append(numpy.linspace(vect_pos[0].p[1], vect_pos[1].p[1], num, endpoint = True, retstep = False, dtype = None))
#    y.append(numpy.linspace(vect_pos[1].p[1], vect_pos[2].p[1], num, endpoint = True, retstep = False, dtype = None))
#    y.append(numpy.linspace(vect_pos[2].p[1], vect_pos[3].p[1], num, endpoint = True, retstep = False, dtype = None))

#    z.append(numpy.linspace(vect_pos[0].p[2], vect_pos[1].p[2], num, endpoint = True, retstep = False, dtype = None))
#    z.append(numpy.linspace(vect_pos[1].p[2], vect_pos[2].p[2], num, endpoint = True, retstep = False, dtype = None))
#    z.append(numpy.linspace(vect_pos[2].p[2], vect_pos[3].p[2], num, endpoint = True, retstep = False, dtype = None))

#    for i in range(0,3):
#        for j in range(0,num):
#            x_points.append(x[i][j])
#            y_points.append(y[i][j])
#            z_points.append(z[i][j])

#    for i in range(0,3*num - 1):
#            if((x_points[i] == x_points[i+1])):
#                x_points[i]= x_points[i] + 0.00000000000001
#            if((y_points[i] == y_points[i+1])):
#                y_points[i]= y_points[i] + 0.00000000000001            
#            if((z_points[i] == z_points[i+1])):
#                z_points[i]= z_points[i] + 0.00000000000001

#    tck2, u = scipy.interpolate.splprep([x_points,y_points,z_points], k=5)
#    u_fine = numpy.linspace(0,1,100)
#    x_fine, y_fine, z_fine = scipy.interpolate.splev(u_fine, tck2)#


#    return x_fine, y_fine, z_fine, x_points, y_points, z_points

#not using this function at the moment
#def cubic_interp_3way(grasp_pos1=[], grasp_pos2=[], grasp_pos3=[]):
#    ####Here I find the cubic polynomials between the original waypoints

#    tck1, u = scipy.interpolate.splprep([grasp_pos1,grasp_pos2,grasp_pos3], k=3)
#    u_fine = numpy.linspace(0,1,300)
#    x_fine, y_fine, z_fine = scipy.interpolate.splev(u_fine, tck1)

#    return x_fine, y_fine, z_fine

def build_traj(x_fine=[],y_fine=[],z_fine=[],orientation=[]):
    traj= []
    for i in range(0,len(x_fine)):
        coord= PyKDL.Frame()
        coord.p[0]=x_fine[i]
        coord.p[1]=y_fine[i]
        coord.p[2]=z_fine[i]
        coord.M= orientation
        traj.append(coord)
    return traj

def from_pykdl_to_matrix(pos_pykdl):
    pose_matrix= np.eye(4, dtype=float)
    pose_matrix[0,3] = pos_pykdl.p[0]
    pose_matrix[1,3] = pos_pykdl.p[1]
    pose_matrix[2,3] = pos_pykdl.p[2]
    pose_matrix[0,0] = pos_pykdl.M[0,0]
    pose_matrix[0,1] = pos_pykdl.M[0,1]
    pose_matrix[0,2] = pos_pykdl.M[0,2]
    pose_matrix[1,0] = pos_pykdl.M[1,0]
    pose_matrix[1,1] = pos_pykdl.M[1,1]
    pose_matrix[1,2] = pos_pykdl.M[1,2]
    pose_matrix[2,0] = pos_pykdl.M[2,0]
    pose_matrix[2,1] = pos_pykdl.M[2,1]
    pose_matrix[2,2] = pos_pykdl.M[2,2]

    return pose_matrix


rospy.init_node('main_node', anonymous=True) 

#Create objects
psm3 = Config() #arm object
nodo_obj= Nodo() #Transform message, it has quaternion and vector3 traslation
transform_obj= Transforma() #tranformation object, getting the actual matrix
psm_simplified= MoveGroupPythonIntefaceTutorial() #moveit tutorial object

#Getting the transformation left_tf pose from a topic
rospy.Subscriber('/tf', TFMessage, nodo_obj.trans_callback)

#Through the config methods I define the transformations between frames
psm3.pos()
psm3.master()

#Getting the actual transformation matrix instead of the quaternion, or an error message
transform_obj.trasformation(nodo_obj.left_tf)


T_c_p0 = np.dot(psm3.T_c_l, np.dot(transform_obj.T_l_o, np.dot(psm3.T_o_pb, psm3.T_pb_p0))) #transformation from fulcrum to camera frame
T_p0_c= np.linalg.inv(T_c_p0) #transformation from camera frame to fulcrum
#T_o_p0 = np.dot(psm3.T_o_pb, psm3.T_pb_p0) # I do not need it at the moment, it is the transform from the fulcrum to the point o on the checkerboard
#T_c_p0= np.dot(self.T_c_l*self.T_o_pb) #I do not need it

psm3= dvrk.psm(psm3.psmName) #this is a dvrk API class from psm.py
#psm1= dvrk.psm('PSM1')  # this is the first arm
#psm2= dvrk.psm('PSM2')  # this is the second arm



#############modifica per non rompere il robot un altra volta.
sleep(3) #to see if a can publish the base frame now

#psm3.set_joint_velocity_ratio(0.2) 
#psm3.set_joint_acceleration_ratio(0.2)

#############



psm3.set_base_frame(T_c_p0) #this is the function I've Implemented to publish the base frame, but before you have to run the broadcaster in broadcaster_monel ( not sure, cannot verify it with the robot disconnected)

#getting the cartesian position respect the base frame that I've set
curpos= PyKDL.Frame()
curpos.p= psm3.get_current_position().p
curpos.M= psm3.get_current_position().M

#I fix the vertical position of the tip, which should be pointing toward the z of the sdr of the camera
curpos_vertical= PyKDL.Frame()
curpos_vertical.p= curpos.p
for i in range(0,2):
    for j in range(0,2):
        curpos_vertical.M[i,j]=0.0
        if(i==0 and j==2):
            curpos_vertical.M[i,j]=-1.0
        if(i==1 and j==0):
            curpos_vertical.M[i,j]= 1.0
        if(i==2 and j==1):
            curpos_vertical.M[i,j]= 1.0

#first movement, it moves the psm in that base configuration, and I close the eef

#psm3.move(curpos_vertical, interpolate = True, blocking = False) 
#psm3.close_jaw()


#here I save the current position of the psm, which is the vertical position parallel to z axis of the camera. Then I save just the rotation matrix, not all the frame
vertical_rotation_global= PyKDL.Frame()
vertical_rotation_global.M= psm3.get_current_position().M


#Now i'm gonna find the coordinates x,y,z of the grasping point, the tissue point and the background point
point= Punti()
print('Getting the points coordinates...') # you have to run the dummy point node before
rospy.Subscriber('/endoscope/disparity/grasping_point', PointStamped, point.grasp_callback)
sleep(2)
rospy.Subscriber('/endoscope/disparity/tissue_point', PointStamped, point.tissue_callback)
sleep(2)
rospy.Subscriber('/endoscope/disparity/background_point', PointStamped, point.background_callback)
sleep(2)

original_point_x= [point.grasp_pt.x, point.tissue_pt.x, point.background_pt.x]
original_point_y= [point.grasp_pt.y, point.tissue_pt.y, point.background_pt.y]
original_point_z= [point.grasp_pt.z, point.tissue_pt.z, point.background_pt.z]


pos=np.array([[point.tissue_pt.x, point.tissue_pt.y], [point.grasp_pt.x, point.grasp_pt.y], [point.background_pt.x, point.background_pt.y] ])


pos_x=np.array([pos[0:2,0]]) #gettint x coordinates of tissue and grasping points
pos_y=np.array([pos[0:2,1]]) #gettint y coordinates of tissue and grasping points
pos_x=np.linspace(pos_x[0,0], pos_x[0,1], num=5)
pos_y=np.linspace(pos_y[0,0], pos_y[0,1], num=5)


#I find the straight line function that connects tissue and grasping point, then i take as a first waypoint a point on that line between the grasping
#and the background
motion=scipy.interpolate.interp1d(pos_x, pos_y, kind='linear', fill_value="extrapolate")
dist=np.linalg.norm(pos[1,:]-pos[2,:]) # i want the shift to be proportional to the distance between background and grasping point
dist= dist*0.3  #want to move into background of 20% or 30% of distance between the background and the grasping point


#compute x and y of first point
if((pos[1,0]-pos[0,0])>0): #caso del flap a sinistra
    q=motion(0.0)
    m=(motion(4.0)-q)/4.0
    xp= pos[1,0] + dist/(math.sqrt(1+math.pow(m,2))) #formula to obtain distance of a point on a straight line
    yp= motion(xp)
else:
    q=motion(0) #flap a destra
    m=(motion(4.0)-q)/4.0
    xp= pos[1,0] - dist/(math.sqrt(1+math.pow(m,2)))
    yp= motion(xp)


grasp_pos_1= PyKDL.Frame()
grasp_pos_2= PyKDL.Frame()
grasp_pos_3= PyKDL.Frame()

vect_pos= []
vect_pos.append(psm3.get_current_position())

#this is the first way-point, moving from original position to this point, xp, yp and the computed z
grasp_pos_1.p[0]=xp
grasp_pos_1.p[1]=yp
grasp_pos_1.p[2]=point.tissue_pt.z*0.5 
grasp_pos_1.M= vertical_rotation_global.M
vect_pos.append(grasp_pos_1)

#this is the second way-point, moving along z, between the z of the tissue and the z of the background
grasp_pos_2.p[0]=xp
grasp_pos_2.p[1]=yp
grasp_pos_2.p[2]=(point.background_pt.z+point.tissue_pt.z)/2 
grasp_pos_2.M= vertical_rotation_global.M
vect_pos.append(grasp_pos_2)

#this is the third way-point, moving along x and y to grasp the tissue from the side
grasp_pos_3.p[0]=point.tissue_pt.x
grasp_pos_3.p[1]=point.tissue_pt.y
grasp_pos_3.p[2]=(point.background_pt.z+point.tissue_pt.z)/2 
grasp_pos_3.M= vertical_rotation_global.M
vect_pos.append(grasp_pos_3)

grasp_pos_x= [vect_pos[0].p[0], vect_pos[1].p[0], vect_pos[2].p[0], vect_pos[3].p[0]]
grasp_pos_y= [vect_pos[0].p[1], vect_pos[1].p[1], vect_pos[2].p[1], vect_pos[3].p[1]]
grasp_pos_z= [vect_pos[0].p[2], vect_pos[1].p[2], vect_pos[2].p[2], vect_pos[3].p[2]]

print('These are the 3 waypoints:')
print(grasp_pos_1)
print(grasp_pos_2)
print(grasp_pos_3)

###So far I've found the 3 original waypoints, now i have to find the polynomial spline interpolation between them


#THis is the python simple interpolation, I'm not actually using it because otherwise i will find smooth profiles of a trajectory which may be not
#feasible for the robot since interpolation function does not know the kinematics of the robot or the presence of obstacles

############################################################################# PYTHON SIMPLE INTERPOLATION - Not using anymore
#x_fine1, y_fine1, z_fine1= cubic_interp_3way(grasp_pos_x, grasp_pos_y, grasp_pos_z)
#x_fine2, y_fine2, z_fine2, x_points, y_points, z_points= quintic_interp_Nway(vect_pos)
#x_fine3, y_fine3, z_fine3= cubic_interp_5way(grasp_pos_x, grasp_pos_y, grasp_pos_z)


#fig = plt.figure()
#ax = Axes3D(fig)
#ax.set_xlabel('X Label')
#ax.set_ylabel('Y Label')
#ax.set_zlabel('Z Label')
#ax.scatter(original_point_x[0], original_point_y[0], original_point_z[0], c='Red', marker='o', label="Grasping Point")
#ax.scatter(original_point_x[1], original_point_y[1], original_point_z[1], c='Green', marker='o', label="Tissue Point")
#ax.scatter(original_point_x[2], original_point_y[2], original_point_z[2], c='Brown', marker='o', label="Background Point")
#ax.scatter(x_points, y_points, z_points, c='Blue', marker='x')
#ax.scatter(grasp_pos_x, grasp_pos_y, grasp_pos_z, c='Black', marker='o', label="3 Original waypoints waypoints")
#ax.plot(x_fine3, y_fine3, z_fine3, c='Red', label="Cubic interpolation using 5 waypoints")
#ax.plot(x_fine1, y_fine1, z_fine1, c='Green',label="Cubic interpolation using 3 waypoints")
#ax.plot(x_fine2, y_fine2, z_fine2, c='Blue', label="Quintic interpolation using N waypoints")
#ax.legend()
#plt.show()


#traj1= build_traj(x_fine1, y_fine1, z_fine1, vertical_rotation_global.M )
#traj2= build_traj(x_fine2, y_fine2, z_fine2, vertical_rotation_global.M )
#traj3= build_traj(x_fine3, y_fine3, z_fine3, vertical_rotation_global.M )
###############################################################################


############################################################################ ADDING OBSTACLES TO THE SCENE


## I have to retrieve the coordinates of the end effector. Theoretically with get current position i should get the position of the wrist tool of 
# the da vinci wrt base frame, which should be in camera frame. I change it into fulcrum frame by multipliying it to the transf matrix that
# i've found. Then I have the DH parameters, so i basically have the trasnf matrix from one joint frame to the other in the urdf file.it is
# in roll pitch raw, i convert it into homogeneous matrix. i multiply the omogeneous matrix of sequential links to the transf matrix, so i find th
#pose of the link respect the base frame, then i convert it into quaternion and add as an obstacle.


#pose_psm1_yaw_tool_wrt_base_frame= from_pykdl_to_matrix(psm1.get_current_position())
#pose_psm2_yaw_tool_wrt_base_frame= from_pykdl_to_matrix(psm2.get_current_position())
#pose_psm1_yaw_tool_wrt_p0= np.dot(T_p0_c, pose_psm1_yaw_tool_wrt_base_frame)
#pose_psm2_yaw_tool_wrt_p0= np.dot(T_p0_c, pose_psm2_yaw_tool_wrt_base_frame)
#psm_simplified.add_obstacles_main_tools(pose_psm1_yaw_tool_wrt_p0)
#psm_simplified.add_obstacles_main_tools(pose_psm2_yaw_tool_wrt_p0)


############################################# Here I assign the pose of the end effector to make it work
# example values, got it by transforming from euler angles to quaternion, and with this function i transform from quaternion to homogeneous matrix

#PSM1
psm1_pose_yaw_link= transformations.quaternion_matrix([0.19134172, 0.46193968, 0.19134172, 0.84462325]) 
psm1_pose_yaw_link[0][3]= 0.05# 0.1
psm1_pose_yaw_link[1][3]= 0.05# 0.2
psm1_pose_yaw_link[2][3]= -0.15# 0.05

#PSM2
psm2_pose_yaw_link= transformations.quaternion_matrix([-0.32664086, -0.29516016, -0.32664086,  0.83635637])
psm2_pose_yaw_link[0][3]= 0.2
psm2_pose_yaw_link[1][3]= 0.3
psm2_pose_yaw_link[2][3]= -0.1

psm_simplified.transformations_matrix() #Here I assign the various transformations matrix between links that i derived from DH parameters
psm_simplified.add_obstacles_main_tools(psm1_pose_yaw_link, 'psm1_') #here i add the obstacles (the psms) to the scene
psm_simplified.add_obstacles_main_tools(psm2_pose_yaw_link, 'psm2_')

#raw_input()
#psm_simplified.remove_obstacles()
###############################################
################################################################# finish to add obstacles



############################################################################### MOVEIT TRAJECTORY
#THis is what i'm using to compute the trajectory through moveIt
#Here I transform the waypoints that I've computed  from the camera frame to the fulcrum frame (the one which is used by moveit)
#I will use these points once i will be able to receive points from the neural network
pos_for_moveIt= []
for i in range(0,len(grasp_pos_x)):
    pos_in_p0_frame= np.dot(T_p0_c,np.array([[grasp_pos_x[i]],[grasp_pos_y[i]],[grasp_pos_z[i]],[1]]))
    pos_for_moveIt.append([pos_in_p0_frame[0,0], pos_in_p0_frame[1,0], pos_in_p0_frame[2,0]])

#############chosen point in the fulcrum frame to simulate the movement of insertion
pos_for_moveIt[0]= [0.02, 0.02, -0.07] #initial position
pos_for_moveIt[1]= [0.03, 0.03, -0.10]
pos_for_moveIt[2]= [0.08, 0.05, -0.20]
pos_for_moveIt[3]= [0.09, 0.05, -0.20] #final position
############
rate= 200 #frequency chosen to communicate with the actuators


# I set the rate (200) that I want the actuators to communicate with, i set the constant velocity(3.0 cm) that i want. I send to moveit these two values and i calculcate
#the trajectory. in moveit_node i will compute the trajectories between the points, the interpolation between them, then i will compute the distance covered by
# the trajectory and, basing on the velocity that i want, i find the duration of the movement.
#Through the movement and the rate i find the desired number of samples, and I interpolate the trajectory with the right number of samples.
#then i have to publish these points at that rate on the dvrk move topic
#assume that I want an average velocity of 3.0 cm/second and a rate of 200Hz to communicate with the actuators
vel= 3.0
rate= 200
#i get the smooth trajectories, the duration, the number of samples, the distance covered
traj_moveIt, duration, numsamples, distance = psm_simplified.motion_interp_main_joint(pos_for_moveIt, rate, vel)


#Here I convert the moveit trajectory in a pykdl traj, but before I also have to reconvert the single points in the camera frame
x_after_moveIt= []
y_after_moveIt= []
z_after_moveIt= []
for i in range(0,len(traj_moveIt)):
    point_after_moveIt= np.dot(T_c_p0,np.array([[traj_moveIt[i][0]],[traj_moveIt[i][1]],[traj_moveIt[i][2]],[1]]))
    x_after_moveIt.append(point_after_moveIt[0,0])
    y_after_moveIt.append(point_after_moveIt[1,0])
    z_after_moveIt.append(point_after_moveIt[2,0])

traj_after_moveIt= build_traj(x_after_moveIt, y_after_moveIt, z_after_moveIt, vertical_rotation_global.M )


######################################################################################### finish to compute trajectory



#summary of the trajectory
print('The length of the path is: '+str(distance)+' m\nThe chosen average velocity is: '+str(vel)+' cm/sec\nThe resulting duration of the movement is: '+str(duration)+' s\nThe chosen rate is: '+str(rate)+' Hz\nThe number of samples is: '+str(numsamples))
psm_simplified.remove_obstacles()
#raw_input()



########################################################## HERE I ACTUALLY EXECUTE THE TRAJECTORY
i=0
error=[]
#while not rospy.is_shutdown() and i<len(traj_after_moveIt):
    #psm3.move(traj_after_moveIt[i], interpolate = False)
    #I have to check this error thing, i will probably compure the diff between current position e traj[i]
    #errorX = traj_after_moveIt[i].p[0] - self.arm.get_current_position().p[0]
    #errorY = traj_after_moveIt[i].p[1] - self.arm.get_current_position().p[1]
    #errorZ = traj_after_moveIt[i].p[2] - self.arm.get_current_position().p[2]
    #error.append(math.sqrt(errorX * errorX + errorY * errorY + errorZ * errorZ))
    #if error > 0.002: # 2 mm
    #    print('Inverse kinematic error in position [', i, ']: ', error[i])
    #i=i+1
    #get_current_twist_body to retrive the cartesian velocity of the arm
    #rospy.sleep(1.0 / rate)
############################################################################################################################### END OF THE CODE





#The robot made that wrong movement for 2 reasons: 1 it executed a trajectory which was not feasible for its kinematics. I was not using moveit in that
#moment, I was just interpolating with python functions. So i was not able to distinguish feasibile and not feasibile trajectories.
#In particular, the trajectory was expected to make a first movement of the eef of some cm on the right wrt fulcrum, which is obviously a not feasible
#movement, so it suddenly reached finecorsa.
#Furthermore, i set a frequency which was to high (200hz) for that number of samples (200 or 300), so the robot should have made a movement in just a 1
#second. So it was going really fast following a not feasible trajectory.





