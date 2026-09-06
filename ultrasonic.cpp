#include <Arduino.h>

#include "ultrasonic.h"
#include "config.h"

void initUltrasonic()
{
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    digitalWrite(TRIG_PIN, LOW);
}

long readDistance()
{
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);

    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    long duration = pulseIn(
        ECHO_PIN,
        HIGH,
        30000
    );

    if (duration == 0)
    {
        return -1;
    }

    long distance = duration * 0.0343 / 2;

    return distance;
}