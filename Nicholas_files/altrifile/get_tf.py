#!/usr/bin/env python


import rospy
from geometry_msgs.msg import Transform

class Nodo:
    def __init__(self):
        self.left_tf = Transform()
    def trans_callback(self, data):
        if(data.transforms[0].child_frame_id == 'PSM3'):
            #if(data.transforms.child_frame_id == 'checkerboard'): deve essere cosi in realta
            self.left_tf = data.transforms[0].transform




#def get_transf(self):
#    rospy.Subscriber('/tf', TFMessage, nodo_obj.trans_callback)
#    rospy.Subscriber('/tf', Pose, trans_callback)
#    rospy.spin()


	#if __name__ == '__main__':
    #	get_tf()
