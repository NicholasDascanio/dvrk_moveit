#!/usr/bin/env python

import rospy
from geometry_msgs.msg import Point

class Punti:
    def __init__(self):
        self.grasp_pt= Point()
        self.tissue_pt= Point()
        self.background_pt= Point()

    def grasp_callback(self, data):
        self.grasp_pt= data.point
    def tissue_callback(self, data):
        self.tissue_pt= data.point

    def background_callback(self, data):
        self.background_pt= data.point

    #qui sono un po indeciso perche non so che tipo di messaggio e quello che viaggia su quei topic. dovrebbe essere pointstamped

