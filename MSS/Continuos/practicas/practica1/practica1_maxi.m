% Modelado y Simulación de Sistemas
% Práctica 1 - Modelado y Simulación de Ec. Diferenciales

close all;
clear all;
clc;

% Parámetros del sistema
M1 = 8; % Masa del primer objeto (kg)
M2 = 10; % Masa del segundo objeto (kg)
K1 = 3; % Cte. elástica (N/m)
K2 = 6; % Cte. elástica (N/m)
K3 = 7; % Cte. elástica (N/m)
B1 = 4; % Cte. amortiguación (N·s/m)
B2 = 5; % Cte. amortiguación (N·s/m)
F = 1; % Fuerza externa (N)

% Condiciones iniciales
t_span = [0, 120]; % Intervalo de tiempo
x1_ini = [0; 0];
x2_ini = [0; 0];
init_cond = [x1_ini; x2_ini]; % Posición y velocidad de la masa para t=0

% Solver Ec. Diferencial
[t, y] = ode45(@(t, y) odefcn(t,y,M1,M2,F,B1,B2,K1,K2,K3), t_span, init_cond);

x1_pos = y(:, 1);
x1_vel = y(:, 2);
x2_pos = y(:, 3);
x2_vel = y(:, 4);

% Visualización de resultados
figure;
grid on;
subplot(2, 1, 1);
plot(t, x1_pos, 'b-', t, x2_pos, 'r-');
xlabel('Tiempo (s)');
ylabel('Posición (m)');
legend('x1_pos', 'x2_pos');
subplot(2, 1, 2);
plot(t, x1_vel, 'b-', t, x2_vel, 'r-');
xlabel('Tiempo (s)');
ylabel('Velocidad (m/s)');
legend('x1_vel', 'x2_vel');


function dydt = odefcn(t,y,M1,M2,F,B1,B2,K1,K2,K3)
    % y(1)=x (posicion), y(2)=dx/dt (velocidad)

    dy1dt = [y(2); 1/M1*(F + K2*y(3) - (K1+K2)*y(1) - B1*y(2))];
    dy2dt = [y(4);1/M2*(K2*y(1)-K3*y(3)-K2*y(3)-B2*y(4))];
       
    dydt = [dy1dt; dy2dt];
end