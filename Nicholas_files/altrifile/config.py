#!/usr/bin/env python


import rospy
import numpy as np
import math
#import dvrk

class Config:
    def __init__(self):
        self.psmName = 'PSM3'

    def pos(self, L_arm_rot = 40, R_arm_rot=0, r_o_pb = np.array([[-0.141,-0.382,0.040]])):
        self.L_arm_rot = L_arm_rot
        self.R_arm_rot = R_arm_rot
        self.r_o_pb = r_o_pb
        self.T_pb_p0 = np.array([[0,1,0,0.4864], [-1,0,0,0], [0,0,1,0.1524],[0,0,0,1]])
        self.R_o_pb= np.array([[np.cos(math.radians(L_arm_rot)), -np.sin(math.radians(L_arm_rot)), 0], [np.sin(math.radians(L_arm_rot)), np.cos(math.radians(L_arm_rot)), 0 ], [0,0,1]])
        self.T_o_pb= np.eye(4, dtype=float)
        self.T_o_pb[0:3,0:3]= self.R_o_pb
        self.T_o_pb[0:3,3]= self.r_o_pb
        self.T_c_l=np.eye(4, dtype=int)

    def master(self, master_ip='129.11.176.75', master_port= 11311):
        self.ip = master_ip
        self.port = master_port


#if __name__ == '__main__':
#   try:
#        Config()
#   except rospy.ROSInterruptException:
#        pass