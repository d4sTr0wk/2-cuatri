% Modelado y Simulación de Sistemas
% Práctica 1 - Modelado y Simulación de Ec. Diferenciales

% Parámetros del sistema
M1 = 8; % Masa del primer objeto (kg)
M2 = 10; % Masa del segundo objeto (kg)
K1 = 3; % Cte. elástica (N/m)
K2 = 6; % Cte. elástica (N/m)
K3 = 7; % Cte. elástica (N/m)
B1 = 4; % Cte. amortiguación (N·s/m)
B2 = 5; % Cte. amortiguación (N·s/m)
f = 1; % Fuerza externa (N)

% Condiciones iniciales
t_span = [0, 120]; % Intervalo de tiempo
init_cond = [0; 0]; % Posición y velocidad de la masa para t=0

% Solver Ec. Diferencial
[t, y] = ode45(@(t, y) odefcn(t,y,M1,f,B1,K1,K2), t_span, init_cond);

x_pos = y(:, 1);
x_vel = y(:, 2);

% Visualización de resultados
figure;
grid on;
subplot(2, 1, 1);
plot(t, x_pos, 'b-');
xlabel('Tiempo (s)');
ylabel('Posición (m)');
legend('x1');
subplot(2, 1, 2);
plot(t, x_vel, 'b-');
xlabel('Tiempo (s)');
ylabel('Velocidad (m/s)');
legend('x1');


function dydt = odefcn(t,y,M1,f,B1,K1,K2)
    % y(1)=x (posicion), y(2)=dx/dt (velocidad)

    % dydt(1) = dx/dt = y(2)
    % dydt(2) = 
    dydt = [y(2); f/M1 + K2/M1*y(1) - (K1+K2)/M1*y(1) - B1/M1*y(2)];
end