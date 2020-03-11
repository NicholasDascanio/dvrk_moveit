
load traj_joint.mat

pos= datajointmodified(1:31,:);
vel= datajointmodified(33:63,:);
acc= datajointmodified(65:end,:);

%% Pos, acc, vel for each joint in different graphs
subplot(3,1,1);
p = stackedplot(pos);
p.Color= 'red';
title('Positions values');
subplot(3,1,2);
v = stackedplot(vel);
v.Color= 'blue';
title('Velocities values');
subplot(3,1,3);
a = stackedplot(acc);
a.Color= 'green';
title('Accelerations values');
%% Traj for each joint in the same graph

figure;
subplot(3,3,1);
plot(pos{:,1}, 'red');
hold on
plot(vel{:,1}, 'blue');
hold on
plot(acc{:,1}, 'green');
title('joint1');

subplot(3,3,2);
plot(pos{:,2}, 'red');
hold on
plot(vel{:,2}, 'blue');
hold on
plot(acc{:,2}, 'green');
title('joint2');

subplot(3,3,3);
plot(pos{:,3}, 'red');
hold on
plot(vel{:,3}, 'blue');
hold on
plot(acc{:,3}, 'green');
title('joint3');

subplot(3,3,4);
plot(pos{:,4}, 'red');
hold on
plot(vel{:,4}, 'blue');
hold on
plot(acc{:,4}, 'green');
title('joint4');

subplot(3,3,5);
plot(pos{:,5}, 'red');
hold on
plot(vel{:,5}, 'blue');
hold on
plot(acc{:,5}, 'green');
title('joint5');
legend('Positions', 'Velocities', 'Accelerations');

subplot(3,3,6);
plot(pos{:,6}, 'red');
hold on
plot(vel{:,6}, 'blue');
hold on
plot(acc{:,6}, 'green');
title('joint6');

subplot(3,3,7);
plot(pos{:,7}, 'red');
hold on
plot(vel{:,7}, 'blue');
hold on
plot(acc{:,7}, 'green');
title('joint7');

%% Pos, acc, vel for each joint in the same graph

figure;
subplot(3,1,1);
plot(pos{:,:})
title('Positions Values for each joint')
legend('joint1','joint2','joint3','joint4','joint5','joint6','joint7');
subplot(3,1,2);
plot(vel{:,:})
title('Velocities Values for each joint')
legend('joint1','joint2','joint3','joint4','joint5','joint6','joint7');
subplot(3,1,3);
plot(acc{:,:})
title('Accelerations Values for each joint')
legend('joint1','joint2','joint3','joint4','joint5','joint6','joint7');
