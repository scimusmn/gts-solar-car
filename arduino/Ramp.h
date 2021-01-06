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
  int pwmPerInterval;
  unsigned long currentMillis, millisLastChange;
  int percentToPWM(int);

public:
  bool isRamping = false;
  Ramp(bool, int);      // isActiveHigh, pin, time (millis to go 0 to 100%)
  void setPercent(int); //immediately set to percent 0-100
  int getPercent();
  void rampTo(int, long);
  void update();
};

Ramp::Ramp(bool _isActiveHigh = true, int _pin = 13)
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
  return map(currentDutyCycle, 0, 255, 0, 100);
}

void Ramp::rampTo(int percent, long time)
{
  targetDutyCycle = percentToPWM(percent);

  if (currentDutyCycle == targetDutyCycle)
    return;

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
    pwmPerInterval = delta / time;
  }

  if (delta < time)
  {
    pwmPerInterval = 1;
    Serial.println(time);
    Serial.println(delta);
    interval = time / delta;
  }

  if ((targetDutyCycle - currentDutyCycle) < 0)
    pwmPerInterval = -pwmPerInterval;

  Serial.print("interval: ");
  Serial.println(interval);
  Serial.print("pwm change per interval: ");
  Serial.println(pwmPerInterval);
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
      currentDutyCycle += (pwmPerInterval * numIntervals);
      if (pwmPerInterval > 0)
        currentDutyCycle = constrain(currentDutyCycle, 0, targetDutyCycle);
      else if (pwmPerInterval < 0)
        currentDutyCycle = constrain(currentDutyCycle, targetDutyCycle, 255);
      analogWrite(pwmPin, currentDutyCycle);
      if (targetDutyCycle == currentDutyCycle)
        isRamping = false;
      // Serial.print("current:");
      // Serial.println(currentDutyCycle);
      // Serial.print("target:");
      // Serial.println(targetDutyCycle);
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