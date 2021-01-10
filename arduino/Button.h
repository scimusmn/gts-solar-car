#include "Arduino.h"

// ensure this library description is only included once
#ifndef Button_h
#define Button_h

class Button
{
public:
  bool state;
  bool fired;
  bool lastFired;
  unsigned long debounceTimer;
  int debounce;
  int pin;
  int lightPin;
  void (*callback)(int state);

  Button() {}

  void setup(int p, int l, void (*CB)(int))
  {
    callback = CB;
    lightPin = l;
    pin = p;
    pinMode(p, INPUT_PULLUP);
    pinMode(lightPin, OUTPUT);
    digitalWrite(lightPin, LOW);
    debounceTimer = 0;
    debounce = 20;
    lastFired = state = fired = true;
    digitalWrite(lightPin, LOW);
  }

  void update()
  {
    if (digitalRead(pin) != state)
    {
      state = !state;
      fired = !state;
      debounceTimer = millis() + debounce;
    }

    if (debounceTimer < millis() && state != fired && lastFired != state)
    {
      lastFired = fired = state;
      callback(!state);
    }
  }

  void lightOff()
  {
    digitalWrite(lightPin, LOW);
  }
  void lightOn()
  {
    digitalWrite(lightPin, HIGH);
  }
};

#endif
