#ifndef MOTORS_H
#define MOTORS_H

void initMotors();

void moveForward();
void moveBackward();
void stopMotors();

// Giro en el sitio (motores en sentidos opuestos).
void turnLeft();
void turnRight();

// Mueve el robot una distancia en cm.
// forward = true  -> hacia adelante
// forward = false -> hacia atrás
// Devuelve true si terminó, false si hubo timeout o distancia inválida.
bool moveDistanceCm(bool forward, float cm);

// Rota el robot en el sitio durante `ms` milisegundos.
// left = true  -> gira a la izquierda
// left = false -> gira a la derecha
// Devuelve true si terminó normalmente.
bool turnMs(bool left, unsigned long ms);

#endif
