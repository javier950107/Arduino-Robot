#include <Arduino.h>

#include "motors.h"
#include "encoder.h"
#include "config.h"


void initMotors()
{
    pinMode(MOTOR_L_IN1, OUTPUT);
    pinMode(MOTOR_L_IN2, OUTPUT);

    pinMode(MOTOR_R_IN1, OUTPUT);
    pinMode(MOTOR_R_IN2, OUTPUT);

    stopMotors();
}


void moveForward()
{
    digitalWrite(MOTOR_L_IN1, HIGH);
    digitalWrite(MOTOR_L_IN2, LOW);

    digitalWrite(MOTOR_R_IN1, HIGH);
    digitalWrite(MOTOR_R_IN2, LOW);
}


void moveBackward()
{
    digitalWrite(MOTOR_L_IN1, LOW);
    digitalWrite(MOTOR_L_IN2, HIGH);

    digitalWrite(MOTOR_R_IN1, LOW);
    digitalWrite(MOTOR_R_IN2, HIGH);
}


void stopMotors()
{
    digitalWrite(MOTOR_L_IN1, LOW);
    digitalWrite(MOTOR_L_IN2, LOW);

    digitalWrite(MOTOR_R_IN1, LOW);
    digitalWrite(MOTOR_R_IN2, LOW);
}


// Giro en el sitio: rueda izquierda hacia atrás, derecha hacia adelante.
void turnLeft()
{
    digitalWrite(MOTOR_L_IN1, LOW);
    digitalWrite(MOTOR_L_IN2, HIGH);

    digitalWrite(MOTOR_R_IN1, HIGH);
    digitalWrite(MOTOR_R_IN2, LOW);
}


// Giro en el sitio: rueda izquierda hacia adelante, derecha hacia atrás.
void turnRight()
{
    digitalWrite(MOTOR_L_IN1, HIGH);
    digitalWrite(MOTOR_L_IN2, LOW);

    digitalWrite(MOTOR_R_IN1, LOW);
    digitalWrite(MOTOR_R_IN2, HIGH);
}


bool moveDistanceCm(bool forward, float cm)
{
    if (cm <= 0)
    {
        return false;
    }

    long targetPulses = (long)(cm * PULSES_PER_CM);

    if (targetPulses <= 0)
    {
        return false;
    }

    resetEncoders();

    if (forward)
    {
        moveForward();
    }
    else
    {
        moveBackward();
    }

    unsigned long startTime = millis();

    while (true)
    {
        long leftPulses = getLeftPulses();
        long rightPulses = getRightPulses();

        // Promedio para tolerar pequeñas diferencias entre ruedas.
        long averagePulses = (leftPulses + rightPulses) / 2;

        if (averagePulses >= targetPulses)
        {
            break;
        }

        if (millis() - startTime > MOVE_TIMEOUT_MS)
        {
            stopMotors();
            return false;
        }

        delay(5);
    }

    stopMotors();
    return true;
}


bool turnMs(bool left, unsigned long ms)
{
    if (ms == 0)
    {
        return false;
    }

    // Tope de seguridad para que una petición errónea no gire sin fin.
    if (ms > 5000)
    {
        ms = 5000;
    }

    if (left)
    {
        turnLeft();
    }
    else
    {
        turnRight();
    }

    delay(ms);

    stopMotors();
    return true;
}
