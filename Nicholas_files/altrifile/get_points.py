#!/usr/bin/env python

import rospy
from geometry_msgs.msg import Point

class Punti:
    def __init__(self):
        self.grasp_pt= Point()
        self.tissue_pt= Point()
        self.background_pt= Point()

    def grasp_callback(self, data):
        self.grasp_pt.x= data.x
        self.grasp_pt.y= data.y
        self.grasp_pt.z= data.z

    def tissue_callback(self, data):
        self.tissue_pt.x= data.x
        self.tissue_pt.y= data.y
        self.tissue_pt.z= data.z

    def background_callback(self, data):
        self.background_pt.x= data.x
        self.background_pt.y= data.y
        self.background_pt.z= data.z

#def get_grasp_pt(self):
    #rospy.init_node('get_grasp_pt', anonymous=True)
    #rospy.Subscriber('/endoscope/disparity/grasping_point', Point, self.grasp_callback)
    #rospy.spin()
#def get_tissue_pt(self):
    #rospy.init_node('get_tissue_pt', anonymous=True)
    #rospy.Subscriber('/endoscope/disparity/tissue_point', Point, self.tissue_callback)
    #rospy.spin()
#def get_background_pt(self):
    #rospy.init_node('get_background_pt', anonymous=True)
    #rospy.Subscriber('/endoscope/disparity/background_point', Point, self.background_callback)
    #rospy.spin()

    #if __name__ == '__main__':
    #    try:
    #        Punti()
    #    except rospy.ROSInterruptException:
    #        pass
