#!/usr/bin/env python


##Devi costruire il node relativo my_motion
##DEvi scoprire da quanti valori e' l'array di joint
## e' un serial chain?

## BEGIN_SUB_TUTORIAL imports
##
## To use the Python MoveIt interfaces, we will import the `moveit_commander`_ namespace.
## This namespace provides us with a `MoveGroupCommander`_ class, a `PlanningSceneInterface`_ class,
## and a `RobotCommander`_ class. More on these below. We also import `rospy`_ and some messages that we will use
##

import sys
import rospy
import numpy as np
import math
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
from time import sleep
import scipy
#import dvrk
#import PyKDL
from scipy.interpolate import interp1d,splrep
from tf import transformations
from tf2_msgs.msg import TFMessage
from std_msgs.msg import String, Bool, Float32, Empty, Float64MultiArray
from geometry_msgs.msg import Pose, PoseStamped, Vector3, Quaternion, Wrench, WrenchStamped, TwistStamped, Point, Transform
from sensor_msgs.msg import JointState, Joy

from config import Config
from auto_transform import Transforma
from get_tf import Nodo
from get_points import Punti

#from dvrk.arm import *
#oppure from arm import
#I modified arm by adding set_base_frame(self, htrasf) and the relative publisher
#from dvrk.arm import *

from psm import *
#I need psm because I call methods like open and close jaw
#from dvrk.psm import *

#da qui in poi dovrebbe essere il main

rospy.init_node('main_node', anonymous=True) #not sure, it has to be just one?

#Create objects
psml = Config() #arm object
nodo_obj= Nodo() #Transform message, it has quaternion and vector3 traslation
transform_obj= Transforma() #tranformation object, getting the actual matrix

print(nodo_obj.left_tf)
#Getting the transformation left_tf pose from a topic
rospy.Subscriber('/tf', TFMessage, nodo_obj.trans_callback)
raw_input()
#print(nodo_obj.left_tf)
#Through the config methods I define the transformations between frames
psml.pos()
psml.master()
print('eeeeeeeeeee')
print(nodo_obj.left_tf)
#MODIFICA PER FAR RUNNARE*******************
#Just to obtain a left_tf pose without using the callback function
#nodo.left_tf = Pose()
#nodo.left_tf.position.x = 1.0
#nodo.left_tf.position.y = 2.0
#nodo.left_tf.position.z = 3.0
#nodo.left_tf.orientation.x = 4.0
#nodo.left_tf.orientation.y = 5.0
#nodo.left_tf.orientation.z = 6.0
#nodo.left_tf.orientation.w = 7.0
#FINE MODIFICA*****************

#Getting the actual transformation matrix instead of the quaternion, or an error message
transform_obj.trasformation(nodo_obj.left_tf)


T_c_p0 = np.dot(psml.T_c_l, np.dot(transform_obj.T_l_o, np.dot(psml.T_o_pb, psml.T_pb_p0)))
T_o_p0 = np.dot(psml.T_o_pb, psml.T_pb_p0)
#T_c_p0= np.dot(self.T_c_l*self.T_o_pb)

psml= dvrk.psm(psml.psmName) #this is a dvrk API class from psm.py
psml.set_base_frame(T_c_p0) #this is the function I've Implemented to publish the base frame

#getting the cartesian position respect the base frame that I've set
curpos= PyKDL.Frame()
curpos.p= psml.get_current_position().p
curpos.M= psml.get_current_position().M

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
psml.move(curpos_vertical, interpolate = True, blocking = False) #in questa funzione, che e in arm.py, si fa un check su che input gli dai in ingresso. Deve essere di tipo PyKDL altrimenti
#non fa nulla, poi controlla se e un pykdl vector, in quel caso fa la traslazione, se e di tipo rotazione, allora fa la rotazione, se e un frame, allora fa una roto-traslazione.
#associa all oggetto psml __move_rotation nel caso di una rotazione o __move_frame nel caso di roto-tralazione ecc, quindi lo muove
psml.close_jaw()


#here I save the current position of the psm, which is the vertical position parallel to z axis of the camera. Then I save just the rotation matrix, not all the frame
vertical_rotation_global= PyKDL.Frame()
vertical_rotation_global.M= psml.get_current_position().M

#Now i'm gonna find the coordinates x,y,z of the grasping point, the tissue point and the background point
point= Punti()

rospy.Subscriber('/endoscope/disparity/grasping_point', Point, point.grasp_callback)
rospy.Subscriber('/endoscope/disparity/tissue_point', Point, point.tissue_callback)
rospy.Subscriber('/endoscope/disparity/background_point', Point, point.background_callback)


#MODIFICA PER FAR RUNNARE*******************
#point.grasp_pt= Point()
#point.tissue_pt= Point()
#point.background_pt= Point()
#point.grasp_pt.x= 1.0
#point.grasp_pt.y= 2.0
#point.grasp_pt.z= 3.0
#point.tissue_pt.x= 8.0
#point.tissue_pt.y= 9.0
#point.tissue_pt.z= 10.0
#point.background_pt.x= 1.0
#point.background_pt.y= 2.0
#point.background_pt.z= 3.0
#FINE MODIFICA*****************


pos=np.array([[point.tissue_pt.x, point.tissue_pt.y], [point.grasp_pt.x, point.grasp_pt.y]])

pos_x=np.array([pos[0:2,0]])
pos_y=np.array([pos[0:2,1]])
pos_x=np.linspace(pos_x[0,0], pos_x[0,1], num=5)
pos_y=np.linspace(pos_y[0,0], pos_y[0,1], num=5)

motion=scipy.interpolate.interp1d(pos_x, pos_y, kind='linear', fill_value="extrapolate") #devo trovare una funzione seria per interpolare, o altrimenti uso una funzione di quelle che ho gia'
dist=np.linalg.norm(pos[0,1]-pos[1,1])
dist= dist*0.1  #want to move into background tissure of 10% of distance between the points

#compute x and y of first point
if((pos[1,0]-pos[0,0])>0): #caso del flap a sinistra
    coeff=motion(0.0)
    coeff=(motion(5.0)-coeff)/5.0
    xp= pos[1,0] + dist/(math.sqrt(1+math.pow(coeff,2))) #formula per ottenere la distanza di un punto sulla retta. motion quella roba li e il coefficiente angolare
    yp= motion(xp)
else:
    coeff=motion(0)
    coeff=(motion(4.0)-coeff)/4.0
    xp= pos[1,0] - dist/(math.sqrt(1+math.pow(coeff,2)))
    yp= motion(xp)

grasp_pos_1= PyKDL.Frame()
grasp_pos_2= PyKDL.Frame()
grasp_pos_3= PyKDL.Frame()

#this is the first way-point, moving from original position to this point, xp, yp and the computed z
grasp_pos_1.p[0]=xp
grasp_pos_1.p[1]=yp
grasp_pos_1.p[2]=point.tissue_pt.z-(point.tissue_pt.z*0.5)
grasp_pos_1.M= vertical_rotation_global.M

#this is the second way-point, moving along z, between the z of the tissue and the z of the background
grasp_pos_2.p[0]=xp
grasp_pos_2.p[1]=yp
grasp_pos_2.p[2]=(point.background_pt.z+point.tissue_pt.z)/2
grasp_pos_2.M= vertical_rotation_global.M

#this is the third way-point, moving along x and y to grasp the tissue from the side
grasp_pos_3.p[0]=point.tissue_pt.x
grasp_pos_3.p[1]=point.tissue_pt.y
grasp_pos_3.M= vertical_rotation_global.M
grasp_pos_3.p[2]=(point.background_pt.z+point.tissue_pt.z)/2

#Now I theoretically have the 3 waypoints that I need.
#I should generate the trajectory through moveIt using the 3 way-points, then i should convert each cartesian point in a PyKDL
#frame object, and then I should rate between 200Hz and 1kHz using the move commands with Interpolate = False

#rate= rospy.Rate(10000)
#while not rospy.is_shutdown():
#    psml.move(cartesian_traj_points[0], interpolate = False)
#    rate.sleep()























#pos_interp= np.array([[curpos_vertical.p[0], curpos_vertical.p[1], curpos_vertical.p[2]], [grasp_pos_1.p[0], grasp_pos_1.p[1], grasp_pos_1.p[2]], [grasp_pos_2.p[0],grasp_pos_2.p[1],grasp_pos_2.p[2]], [grasp_pos_3.p[0],grasp_pos_3.p[1],grasp_pos_3.p[2]]])
#pos_x=np.array([curpos_vertical.p[0], grasp_pos_1.p[0], grasp_pos_2.p[0], grasp_pos_3.p[0]])
#pos_y=np.array([curpos_vertical.p[1], grasp_pos_1.p[1], grasp_pos_2.p[1], grasp_pos_3.p[1]])
#pos_z=np.array([curpos_vertical.p[2], grasp_pos_1.p[2], grasp_pos_2.p[2], grasp_pos_3.p[2]])
#error= PyKDL.diff(grasp_pos_1,grasp_pos_2)
#error2= PyKDL.diff(grasp_pos_2,grasp_pos_3)

#fig = plt.figure()
#ax = Axes3D(fig)
#ax.set_xlabel('X Label')
#ax.set_ylabel('Y Label')
#ax.set_zlabel('Z Label')
#ax.plot(pos_x, pos_y, pos_z, linewidth=1, color='green')
#ax.scatter(pos_x, pos_y, pos_z, cmap='Blue')
#plt.show()



#psml.move(grasp_pos, interpolate = True, blocking = False)

#time.sleep(2)

#move to same x and y but z lower
#grasp_pos.p= psml.get_current_position().p
#grasp_pos.p[2]=(point.background_pt.z+point.tissue_pt.z)/2 #la z e la media fra la z del background point e la z del tissue point
#la grasp pos p e effetivamente il movimento che voglio fare, che e un movimento di traslazione, perche l'orientamento e sempre lo stesso abbiamo detto. poi da qui dovrei interpolare il punto corrente e la grasp pose, con una interpolazione 2d mi viene da pensare, e poi settare una z ancora piu profonda come final state, che non raggiungo, perche quando sono a posa = grasp pose interrompo cambio final state, interpolando il punto corrente con il punto che definisco qua sotto, cioe con la x e y piu spostate, definisco una retta che li unisce, poi stabilisco come final state quel punto, cioe lo metto nel move, perche' io voglio che venga raggiunto
#psml.move(grasp_pos.p, interpolate = True, blocking = False)

#time.sleep(2)

#grasp_pos.p= psml.get_current_position().p
#grasp_pos.p[0]=point.tissue_pt.x
#grasp_pos.p[1]=point.tissue_pt.y
#psml.move(grasp_pos.p, interpolate = True, blocking = False)

#In addition to the standard Python __main__ check, this catches a rospy.ROSInterruptException exception, which can be thrown by rospy.sleep() and rospy.Rate.sleep() methods when Ctrl-C is pressed or your Node is otherwise shutdown. The reason this exception is raised is so that you don't accidentally continue executing code after the sleep(). 
