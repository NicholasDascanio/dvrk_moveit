
%dataraw=csvread('data_cartesian.csv',1,1)

%for i=1:31
%waypoints(i).position=dataraw(i+5,1:3)
%end

clear all
clc


load final_pose.mat
load initial_pose.mat
load waypoints.mat
load way_x.mat
load way_y.mat
load way_z.mat
load original_waypoints.mat
load time_stamps.mat

%% plotting cartesian traj
figure;
scatter3(way_x,way_y,way_z)
hold on
scatter3(original_waypoints(1).position(1),original_waypoints(1).position(2),original_waypoints(1).position(3), 'filled', 'blue')
hold on
scatter3(original_waypoints(2).position(1),original_waypoints(2).position(2),original_waypoints(2).position(3), 'filled', 'blue')
hold on
scatter3(original_waypoints(3).position(1),original_waypoints(3).position(2),original_waypoints(3).position(3), 'filled', 'blue')
xlabel('x');
ylabel('y');
zlabel('z');
title('Cartesian Trajectory. Waypoints in filled circles');

%% retrieving cartesian velocity and plotting

%devi prendere stamp.nsecs dal final points altrimenti non so cosa ne viene
%fuori, poi lo consideri e-09

for i=2:31
    dx(i)= waypoints(i).position(1) - waypoints(i-1).position(1) ;
    dy(i)= waypoints(i).position(2) - waypoints(i-1).position(2) ;
    dz(i)= waypoints(i).position(3) - waypoints(i-1).position(3) ;
    dt(i)= (time_stamps(i) - time_stamps(i-1))*10.^-09 ;
    
    
    vx(i)= dx(i)/dt(i);
    vy(i)= dy(i)/dt(i);
    vz(i)= dz(i)/dt(i);
    
    v(i-1)= sqrt(vx(i).^2+vy(i).^2+vz(i).^2);
end

figure;
plot(v);
xlabel('Waypoints');
ylabel('Velocity');
title('Cartesian Velocity module');
