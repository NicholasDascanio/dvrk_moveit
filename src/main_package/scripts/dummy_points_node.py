#!/usr/bin/env python



import rospy
from geometry_msgs.msg import Pose, PoseStamped, PointStamped

rospy.init_node('dummy_points_node', anonymous=True)

grasping_pub = rospy.Publisher('/endoscope/disparity/grasping_point', PointStamped, latch = True, queue_size = 1)
tissue_pub = rospy.Publisher('/endoscope/disparity/tissue_point', PointStamped, latch = True, queue_size = 1)
background_pub = rospy.Publisher('/endoscope/disparity/background_point', PointStamped, latch = True, queue_size = 1)

rate = rospy.Rate(10)

grasp_point= PointStamped()
tissue_point= PointStamped()
background_point= PointStamped()

grasp_point.header.frame_id= 'PSM3_base'
grasp_point.point.x= 0.03
grasp_point.point.y= 0.04
grasp_point.point.z= 0.05


tissue_point.header.frame_id= 'PSM3_base'
tissue_point.point.x= 0.04
tissue_point.point.y= 0.07
tissue_point.point.z= 0.1

background_point.header.frame_id= 'PSM3_base'
background_point.point.x= 0.01
background_point.point.y= 0.02
background_point.point.z= 0.02


while not rospy.is_shutdown():
    grasping_pub.publish(grasp_point)
    tissue_pub.publish(tissue_point)
    background_pub.publish(background_point)
    rate.sleep()