close all;
clear all;
clc;

% Parámetros del sistema
M1 = 2;     % Masa de la primera masa (kg)
M2 = 0.1;     % Masa de la segunda masa (kg)
F = 5;     % Fuerza externa (N)
B1 = 5;    % Cte. amortiguacion (N·s/m)
K1 = 2;      % Cte. elastica (N/m)
K2 = 3;      % Cte. elastica (N/m)

% Condiciones iniciales
x1_ini = [0; 0];
x2_ini = [0; 0];
init_cond = [x1_ini; x2_ini];

% Tiempo de simulación
t_span = [0, 50];

% Resoluación Sistema de Ecuaciones Diferenciales
[t, y] = ode45(@(t,y) odefcn(t,y,M1,M2,F,B1,K1,K2), t_span, init_cond);

% Extracción de resultados
x1_pos = y(:, 1);
x1_vel = y(:, 2);
x2_pos = y(:, 3);
x2_vel = y(:, 4);

% Representación posición
figure();
grid on; hold on;
plot(t, x1_pos, 'b-', t, x2_pos, 'r-');
xlabel('Tiempo(s)');
ylabel('Posición(m)');
legend('x1(t)', 'x2(t)');

% Representación velocidad
figure();
grid on; hold on;
plot(t, x1_vel, 'b-', t, x2_vel, 'r-');
xlabel('Tiempo(s)');
ylabel('Velocidad(m/s)');
legend('dx1(t)/dt', 'dx2(t)/dt');

function dydt = odefcn(t,y,M1,M2,F,B1,K1,K2)
    dy1dt = [y(2); 1/M1*(F +K2*y(3) -B1*y(2) -(K1+K2)*y(1))];
    dy2dt = [y(4); 1/M2*(K2*(y(1)-y(3)))];

    dydt = [dy1dt; dy2dt];
end