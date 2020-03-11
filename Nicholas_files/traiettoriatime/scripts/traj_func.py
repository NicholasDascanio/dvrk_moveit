#!/usr/bin/env python

import math
import matplotlib.pyplot as plt
import numpy
import rospy
from std_msgs.msg import Float64

class Traiettor:
	def __init__(self):
		self.x=[]
		self.t=[]
	pass

class Trajectory2:
	def __init__(self, ts = 1.0, vmax = 1.2345 ,amax = 3.4566):
		self.ts = ts
		self.vmax = vmax
		self.amax = amax
		self.x = float(0) #vettore di posizioni
		self.target = 0
		self.v = 0 #vettore di velocita
		self.a = 0 #vettore di accelerazione
		self.t = 0 #vettore di tempo
		self.vn = 0 # next velocity

	def setTarget(self, T): #definisce posizione target
		self.target = T

	def setX(self, x): #definisce posizione current
		self.x = x

	def run(self):
		self.t = self.t + self.ts
		sig = numpy.sign( self.target - self.x ) # direction of move

		tm = 0.5*self.ts + math.sqrt( pow(self.ts,2)/4 - (self.ts*sig*self.v-2*sig*(self.target-self.x)) / self.amax )
		if tm >= self.ts:
			self.vn = sig*self.amax*(tm - self.ts)
			# constrain velocity
			if abs(self.vn) > self.vmax:
				self.vn = sig*self.vmax
		else:
			# done (almost!) with move
			self.a = float(0.0-sig*self.v)/float(self.ts)
			if not (abs(self.a) <= self.amax):
				# cannot decelerate directly to zero. this branch required due to rounding-error (?)
				self.a = numpy.sign(self.a)*self.amax
				self.vn = self.v + self.a*self.ts
				self.x = self.x + (self.vn+self.v)*0.5*self.ts
				self.v = self.vn
				assert( abs(self.a) <= self.amax )
				assert( abs(self.v) <= self.vmax )
				return True
			else:
				# end of move
				assert( abs(self.a) <= self.amax )
				self.v = self.vn
				self.x = self.target
				return False

		# constrain acceleration
		self.a = (self.vn-self.v)/self.ts
		if abs(self.a) > self.amax:
			self.a = numpy.sign(self.a)*self.amax
			self.vn = self.v + self.a*self.ts

		# update position
		#if sig > 0:
		self.x = self.x + (self.vn+self.v)*0.5*self.ts
		self.v = self.vn
		assert( abs(self.v) <= self.vmax )
		#else:
		#	self.x = self.x + (-vn+self.v)*0.5*self.ts
		#	self.v = -vn
		return True
#	def zeropad(self):
#		self.t = self.t + self.ts

#	def prnt(self):
#		print ("%.3f\t%.3f\t%.3f\t%.3f" % self.t, self.x, self.v, self.a )

#	def __str__(self):
#		return "2nd order Trajectory."
