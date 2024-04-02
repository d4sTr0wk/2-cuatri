% Práctica 2

% Parte 1

% a - Para valores 𝑅𝑖=50 Ω y 𝐶𝑖=0.01F representa en Matlab la función
% de transferencia 𝐺𝑐(𝑠) = 𝑣𝑜(𝑠)/𝑣𝑖 (𝑠) en formato zpk

% =============
% A) Comenzamos con varaibles simbólica para tener la expresión completa
% =============

syms R1 R2 R3 R4 C1 C2 s
Gc1 = -((R1*R2*C1*C2)*s^2+(R1*C1+R2*C2)*s+1) / (R1*C2*s);
Gc2 = -R4 / R3;
Gc_sym = Gc1 * Gc2;

% Sustituimos valores numéricos y simplificamos
Gc_sym = simplify( subs(Gc_sym, [R1 R2 R3 R4 C1 C2], [50 50 50 50 0.01 0.01]));

Gczpk = minreal( zpk([-2 -2], 0, 1/2));

Gctf = minreal(tf([1 4 2], [2 0]));

% =============
% B) Bucle unitario
% =============

Gt = tf(1, [1 2 2]);

Gtotal = series(Gt, Gctf);

% Realimentación

Gtotal = minreal(feedback(Gtotal, 1));

% Respuesta temporal del sistema total ante la entrada escalón unitario por
% 10 segundos

figure();
step(Gtotal, 10);
xlabel('Tiempo (s)');
ylabel('Señal Gtotal');
