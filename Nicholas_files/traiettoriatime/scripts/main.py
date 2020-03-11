#!/usr/bin/env python

import math
import matplotlib.pyplot as plt
import numpy
import rospy
from traj_func import Traiettor,Trajectory2
from std_msgs.msg import Float64

def main():
	vmax = 3 # max velocity
	amax = 2 # max acceleration
	ts = 0.1 # sampling time
	TK_SILENCE_DEPRECATION=1

	# uncomment one of these:
	traj = Trajectory2( ts, vmax, amax )

	traj.setX(0) # current position
	traj.setTarget(8) # target position

	# resulting (time, position) trajectory stored here:
	traiettoria= Traiettor()

	pub1 = rospy.Publisher('traiettoriatemp', Float64, queue_size=10)
	pub2 = rospy.Publisher('traiettoriapos', Float64, queue_size=10)

	rospy.init_node('comunica_traie', anonymous=True)

	rate = rospy.Rate(10)
# add zero motion at start and end, just for nicer plots
#Nzeropad = 200
#for n in range(Nzeropad):
#	traj.zeropad()
#	t.append( traj.t )
#	x.append( traj.x ) 

# generate the trajectory
	while not rospy.is_shutdown() and traj.run():
		traiettoria.t.append( traj.t )
		traiettoria.x.append( traj.x )
		rospy.loginfo(traj.t)
		rospy.loginfo(traj.x)
		pub1.publish(traj.t)
		pub2.publish(traj.x)
		rate.sleep()
	traiettoria.t.append( traj.t )
	traiettoria.x.append( traj.x )

#for n in range(Nzeropad):
#	traj.zeropad()
#	t.append( traj.t )
#	x.append( traj.x ) 

#print(numpy.size(t))
#print(numpy.size(x))

	#for i in range(0,numpy.size(traiettoria.t),1):
	#	print("\nRobot in position %f at time %f" % (traiettoria.x[i],traiettoria.t[i]))


if __name__ == '__main__':
    try:
        main()
    except rospy.ROSInterruptException:
        pass
#if __name__ == '__main__':
 #   main()

#print(t) #print time trajectory
#print(x) #print position

# plot position, velocity, acceleration, jerk
#plt.figure()
#plt.subplot(4,1,1)
#plt.title( traj )
#plt.plot( t , x , 'r')
#plt.ylabel('Position, x')
#plt.ylim((-2,1.1*traj.target))

#plt.subplot(4,1,2)
#plt.plot( t[:-1] , [d/ts for d in numpy.diff(x)] , 'g')
#plt.plot( t , len(t)*[vmax] , 'g--')
#plt.plot( t , len(t)*[-vmax] , 'g--')
#plt.ylabel('Velocity, v')
#plt.ylim((-1.3*vmax,1.3*vmax))

#plt.subplot(4,1,3)
#plt.plot( t[:-2] , [d/pow(ts,2) for d in numpy.diff( numpy.diff(x) ) ] , 'b')
#plt.plot( t , len(t)*[amax] , 'b--')
#plt.plot( t , len(t)*[-amax] , 'b--')
#plt.ylabel('Acceleration, a')
#plt.ylim((-1.3*amax,1.3*amax))

#plt.subplot(4,1,4)
#plt.plot( t[:-3] , [d/pow(ts,3) for d in numpy.diff( numpy.diff( numpy.diff(x) )) ] , 'm')
#plt.ylabel('Jerk, j')
#plt.xlabel('Time')

#plt.show()
