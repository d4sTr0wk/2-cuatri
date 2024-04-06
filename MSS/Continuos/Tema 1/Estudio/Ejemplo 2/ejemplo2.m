clear all;
close all;
clc;

% Parámetros del sistema
M1 = 2;
M2 = 0.1;
F = 5;
B1 = 5;
B2 = 2;
B3 = 1;
K = 3;

% Condiciones iniciales
x1_ini = [0; 0];
x2_ini = [0; 0];
init_cond = [x1_ini; x2_ini];

% Intervalo de tiempo
t_span = [0, 50];

% Ecuaciones Diferenciales del sistema
[t, y] = ode45(@(t,y) odefcn(t,y,M1,M2,F,B1,B2,B3,K), t_span, init_cond);

% Extraigo el resultado
x1_pos = y(:, 1);
x1_vel = y(:, 2);
x2_pos = y(:, 3);
x2_vel = y(:, 4);

% Visualización de resultados
figure();
grid on; hold on;
plot(t, x1_pos, 'b-', t, x2_pos, 'r-');
xlabel('Tiempo(s)');
ylabel('Posición (m)');
legend('x1(t)','x2(t)');

% Velocidad
figure();
grid on; hold on;
plot(t, x1_vel, 'b-', t, x2_vel, 'r-');
xlabel('Tiempo(s)');
ylabel('Velocidad (m/s)');
legend('dx1(t)/dt','dx2(t)/dt');

function dydt = odefcn(t,y,M1,M2,F,B1,B2,B3,K)
    dy1dt = [y(2); 1/M1*(F + B2*(y(4) - y(2)) - B1*y(2))];
    dy2dt = [y(4); 1/M2*(B2*(y(2) - y(4)) - B3*y(4) - K*y(3))];

    dydt = [dy1dt; dy2dt];
end