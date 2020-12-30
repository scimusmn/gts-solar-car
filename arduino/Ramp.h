// Provides linear ramping (fade on, fade off) control of a PWM pin 

#include "Arduino.h"

#ifndef Ramp_h
#define Ramp_h

class Ramp {
public:
  Ramp(bool, int, long);

  int targetDutyCycle;  // percentage, a value from 0 to 100.    
  int currentDutyCycle;
  bool isActiveHigh;
  long millisFullSwing; // time in milliseconds to go from 0% to 100% or back.  
  
  void Ramp(bool _isActiveHigh = true, int _pin, long _millisFullSwing) {
    pwmPin = _pin
    isActiveHigh = _isActiveHigh;
    pinMode(output, pwmPin);
    if (_start_low)
      analogWrite(0);
    else
      analogWrite(255);
  }

  void update()

private:
  int pwmPin;
  long changePerMillis
  int currentDutyCycle;
  unsigned long currentMillis, millisLastChange;

  

};

#endif