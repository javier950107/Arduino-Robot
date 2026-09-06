#include <ESP32Servo.h>

#include "servo.h"
#include "config.h"

Servo servo;

int currentServoAngle = 90;

void initServo()
{
    servo.setPeriodHertz(50);

    servo.attach(
        SERVO_PIN,
        500,
        2400
    );

    moveServo(90);
}

bool moveServo(int angle)
{
    if (angle < 0 || angle > 180)
    {
        return false;
    }

    servo.write(angle);
    currentServoAngle = angle;

    return true;
}

int getServoAngle()
{
    return currentServoAngle;
}