#include <Arduino.h>

#include "encoder.h"
#include "config.h"

volatile long leftPulses = 0;
volatile long rightPulses = 0;


void IRAM_ATTR leftEncoderISR()
{
    leftPulses++;
}


void IRAM_ATTR rightEncoderISR()
{
    rightPulses++;
}


void initEncoders()
{
    pinMode(ENCODER_L, INPUT_PULLUP);
    pinMode(ENCODER_R, INPUT_PULLUP);

    attachInterrupt(
        digitalPinToInterrupt(ENCODER_L),
        leftEncoderISR,
        RISING
    );

    attachInterrupt(
        digitalPinToInterrupt(ENCODER_R),
        rightEncoderISR,
        RISING
    );
}


long getLeftPulses()
{
    return leftPulses;
}


long getRightPulses()
{
    return rightPulses;
}


void resetEncoders()
{
    noInterrupts();

    leftPulses = 0;
    rightPulses = 0;

    interrupts();
}