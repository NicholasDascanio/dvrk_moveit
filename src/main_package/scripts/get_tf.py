#!/usr/bin/env python


import rospy
from geometry_msgs.msg import Transform
from tf2_msgs.msg import TFMessage

class Nodo:
    def __init__(self):
        self.left_tf = Transform()
        self.left_tf.rotation.w= 1.0

    def trans_callback(self, data):
        if(data.transforms[0].child_frame_id == 'checkerboard'):
            self.left_tf = data.transforms[0].transform


