#!/usr/bin/env python


import rospy
import numpy as np
import math
import tf
from tf import transformations

class Transforma:
	def trasformation(self, left_tf):
		#i find the transformation between the point o on the checkerboard and the left camera
		left_Rot = transformations.quaternion_matrix([left_tf.rotation.x, left_tf.rotation.y, left_tf.rotation.z, left_tf.rotation.w])
		left_Trasl = np.array([[left_tf.translation.x, left_tf.translation.y, left_tf.translation.z]])
		self.T_l_o= np.eye(4, dtype=float)
		self.T_l_o[0:4,0:4]= left_Rot
		self.T_l_o[0:3,3]= left_Trasl

		if (np.size(self.T_l_o)!=16) or (np.linalg.norm(left_Rot*np.transpose(left_Rot)-np.eye(4, dtype=int))>1*math.exp(-6)):
			print ("error: invhform received malformed homogeneous transform")
		else:
			self.T_o_l= np.linalg.inv(self.T_l_o)
