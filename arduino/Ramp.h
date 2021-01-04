// Provides linear ramping (fade on, fade off) control of a PWM pin

#include "Arduino.h"

#ifndef Ramp_h
#define Ramp_h

class Ramp
{
public:
  Ramp(bool, int, long); // isActiveHigh, pin, acceleration (millis to go 0 to 100%)

  int targetDutyCycle; // percentage, a value from 0 to 100.
  int currentDutyCycle;
  bool isActiveHigh;

  void Ramp(bool _isActiveHigh = true, int _pin, long _millis0to100)
  {
    pwmPin = _pin;
    millis0to100 = _millis0to100;
    isActiveHigh = _isActiveHigh;
    pinMode(pwmPin, OUTPUT);
    if (_start_low)
      analogWrite(pwmPin, 0);
    else
      analogWrite(pwmPin, 255);
  }

  void update()
  {
  }

private:
  int pwmPin;
  long millis0to100; // time in milliseconds to go from 0% to 100% or back.
  long changePerMillis int currentDutyCycle;
  unsigned long currentMillis, millisLastChange;
};

#endif