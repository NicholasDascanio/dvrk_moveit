#!/usr/bin/env python

import math
import matplotlib.pyplot as plt
import numpy
import rospy
from traj_func import Traiettor,Trajectory2
from std_msgs.msg import Float64


def callback2(data):
	rospy.loginfo(rospy.get_caller_id() + '\nPosition: %f\n', data.data)


def callback1(data):
	rospy.loginfo(rospy.get_caller_id() + '\nTime: %f\n', data.data)


def ascolta_traie():
    rospy.init_node('ascolta_traie', anonymous=True)

    print('\nRobot configuration:\n')

    rospy.Subscriber('traiettoriatemp', Float64, callback1)

    rospy.Subscriber('traiettoriapos', Float64, callback2)

    rospy.spin()

if __name__ == '__main__':
    try:
        ascolta_traie()
    except rospy.ROSInterruptException:
        pass
