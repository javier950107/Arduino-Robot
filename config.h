#ifndef CONFIG_H
#define CONFIG_H

// WiFi
#define HOSTNAME "robot"

// WebSocket
#define WEBSOCKET_PORT 81

// Servo
#define SERVO_PIN 18

// Ultrasonido
#define TRIG_PIN 5
#define ECHO_PIN 19


// MOTOR IZQUIERDO
#define MOTOR_L_IN1 25
#define MOTOR_L_IN2 26

// MOTOR DERECHO
#define MOTOR_R_IN1 27
#define MOTOR_R_IN2 14


// ENCODERS
// Usa GPIOs con pull-up interno (evita 34/35/36/39 que son input-only).
#define ENCODER_L 32
#define ENCODER_R 33


// Calibración del movimiento por cm.
// Ajusta este valor midiendo cuántos pulsos genera el encoder
// cuando la rueda avanza 1 cm sobre el piso.
// Ejemplo: 20 pulsos por vuelta y perímetro de rueda 20 cm -> 1.0 pulso/cm.
#define PULSES_PER_CM 1.0f

// Tiempo máximo (ms) que un movimiento por cm puede durar antes de abortar.
#define MOVE_TIMEOUT_MS 8000

#endif
