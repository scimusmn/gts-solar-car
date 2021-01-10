// Provides feedback of Encoder position.
#ifndef Encoder_h
#define Encoder_h

#include "Arduino.h"

class Encoder
{
private:
    int encoderPin;        //beam break reading slots on encoder.
    int hallPin;           // hall sensor placed at 0 degrees.
    int encoderResolution; //angular width (degrees) of slot (assume equal width between slots) on encoder wheel.
    int position = 0;
    bool encoderState;
    bool lastEncoderState;
    bool hallState;
    bool lastHallState;
    int slots;

public:
    Encoder(int, int, int); //encoder pin, hall pin, encoder slots per rotation.
    void update();
    int getPositionDeg();
    bool leftFinish;
};

Encoder::Encoder(int _encoderPin, int _hallPin, int _slots) // encoder pin, hall pin, encoder slots per rotation.
{
    encoderPin = _encoderPin;
    hallPin = _hallPin;
    pinMode(encoderPin, INPUT);
    pinMode(encoderPin, INPUT_PULLUP);
    encoderResolution = 360 / (slots * 2);
    slots = _slots;
}

void Encoder::update()
{
    hallState = digitalRead(hallPin);
    if (!hallState)
    {
        position = 0;
        return;
    }
    encoderState = digitalRead(encoderPin);
    if (encoderState != lastEncoderState)
    {
        position++;
        if (position >= (slots * 2))
            position = 0;
        lastEncoderState = encoderState;
    }
    if (position > 0)
        leftFinish = true;
    return;
}

int Encoder::getPositionDeg()
{
    int degrees = encoderResolution * position;
    return degrees;
}

#endif