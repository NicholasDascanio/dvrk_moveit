#!/usr/bin/env python


import rospy
import roslib
from time import sleep
roslib.load_manifest('main_package')
import tf
from geometry_msgs.msg import Pose, PoseStamped

#def callback_monella(data):
#    br = tf.TransformBroadcaster()
#    tp_posi= (data.position.x,data.position.y,data.position.z)
#    tp_orie= (data.orientation.x,data.orientation.y,data.orientation.z,data.orientation.w)
#    br.sendTransform(tp_posi,tp_orie,rospy.Time(),'checkerboard','world') #in realta questo dovrebbe essere fra PSM3_base (credo) oppure PSM3 come parent e checkerboard come child


    
#rospy.init_node('broadcaster_monel')
#rospy.Subscriber('/dvrk/PSM3/set_base_frame', Pose, callback_monella)
#rospy.spin()

class Posa_before:
    def __init__(self):
        self.trasf= Pose()
        

    def callback_monel(self, data):
        self.trasf.position= data.position
        self.trasf.orientation= data.orientation


rospy.init_node('broadcaster_monel')
posa= Posa_before()
rospy.Subscriber('/dvrk/PSM3/set_base_frame', Pose, posa.callback_monel)
sleep(10)
print(posa.trasf)
br = tf.TransformBroadcaster()
rate = rospy.Rate(10.0)
tp_posi= (posa.trasf.position.x,posa.trasf.position.y,posa.trasf.position.z)
tp_orie= (posa.trasf.orientation.x,posa.trasf.orientation.y,posa.trasf.orientation.z,posa.trasf.orientation.w)
while not rospy.is_shutdown():
    br.sendTransform(tp_posi,tp_orie, rospy.Time.now(), "checkerboard", "world")
    rate.sleep()