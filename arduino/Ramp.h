// Provides ramping control of a PWM pin
#ifndef Ramp_h
#define Ramp_h

#include "Arduino.h"

class Ramp
{
private:
  int targetDutyCycle; // 0 thru 255
  int currentDutyCycle;
  bool isActiveHigh;
  int pwmPin;
  int interval;
  int changePerInterval;
  unsigned long currentMillis, millisLastChange;
  int percentToPWM(int);

public:
  bool isRamping = false;
  Ramp(bool, int);        // isActiveHigh, pin
  void setPercent(int);   //immediately set to percent 0-100
  int getPercent();       //returns the percent it's currently at
  void rampTo(int, long); //percent and how many millis to get there.
  void update();          //place in loop
};

Ramp::Ramp(bool _isActiveHigh = true, int _pin = 3)
{
  isActiveHigh = _isActiveHigh;
  pwmPin = _pin;
  pinMode(pwmPin, OUTPUT);
  analogWrite(pwmPin, percentToPWM(0));
  currentDutyCycle = targetDutyCycle = percentToPWM(0);
}

void Ramp::setPercent(int percent) // convenience function to instantly set the percentage
{
  this->rampTo(percent, 0);
}

int Ramp::getPercent()
{
  int percent;
  percent = map(currentDutyCycle, 0, 255, 0, 100);
  if (!isActiveHigh)
    percent = 100 - percent;
  return percent;
}

void Ramp::rampTo(int percent, long time)
{
  targetDutyCycle = percentToPWM(percent);

  if (currentDutyCycle == targetDutyCycle)
  {
    isRamping = false;
    return;
  }

  if (time == 0)
  {
    currentDutyCycle = targetDutyCycle;
    analogWrite(pwmPin, currentDutyCycle);
    isRamping = false;
    return;
  }

  int delta = abs(targetDutyCycle - currentDutyCycle);
  isRamping = true;
  millisLastChange = millis();

  if (delta > time)
  {
    interval = 1;
    changePerInterval = delta / time;
  }
  else if (delta < time)
  {
    changePerInterval = 1;
    interval = time / delta;
  }

  // if delta was negative
  if ((targetDutyCycle - currentDutyCycle) < 0)
    changePerInterval = -changePerInterval;
}

void Ramp::update()
{
  if (!isRamping)
    return;
  else
  {
    currentMillis = millis();

    if ((currentMillis - millisLastChange) >= interval)
    {
      int numIntervals = (currentMillis - millisLastChange) / interval;
      currentDutyCycle += (changePerInterval * numIntervals);

      //prevent overshooting.
      if (changePerInterval > 0)
        currentDutyCycle = constrain(currentDutyCycle, 0, targetDutyCycle);
      else if (changePerInterval < 0)
        currentDutyCycle = constrain(currentDutyCycle, targetDutyCycle, 255);
      analogWrite(pwmPin, currentDutyCycle);
      if (targetDutyCycle == currentDutyCycle)
        isRamping = false;
      millisLastChange = currentMillis;
    }
  }
}

int Ramp::percentToPWM(int percent)
{
  percent = constrain(percent, 0, 100);
  int dutyCycle;
  if (isActiveHigh)
    dutyCycle = map(percent, 0, 100, 0, 255);
  else
    dutyCycle = map(percent, 100, 0, 0, 255);
  return dutyCycle;
}

#endif