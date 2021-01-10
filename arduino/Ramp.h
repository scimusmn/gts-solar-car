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
  void calcRampSpeed(int, long);

public:
  bool isRamping = false;
  Ramp(bool, int);             // isActiveHigh, pin
  void setFullRampSpeed(long); //time (millis) for full swing
  void setPercent(int);        //immediately set to percent 0-100
  int getPercent();            //returns the percent it's currently at
  void rampTo(int, long);      //percent and how many millis to get there.
  void rampTo(int);            //percent to get to using the set RampSpeed;
  void update();               //place in loop
};

Ramp::Ramp(bool _isActiveHigh = true, int _pin = 3)
{
  isActiveHigh = _isActiveHigh;
  pwmPin = _pin;
  pinMode(pwmPin, OUTPUT);
  analogWrite(pwmPin, percentToPWM(0));
  currentDutyCycle = targetDutyCycle = percentToPWM(0);
}

void Ramp::setPercent(int percent) // instantly set the percentage
{
  targetDutyCycle = percentToPWM(percent);
  currentDutyCycle = targetDutyCycle;
  analogWrite(pwmPin, currentDutyCycle);
  isRamping = false;
  return;
}

void Ramp::setFullRampSpeed(long time) //time for full swing
{
  calcRampSpeed(percentToPWM(100), time);
}

int Ramp::getPercent()
{
  int percent;
  percent = map(currentDutyCycle, 0, 255, 0, 100);
  if (!isActiveHigh)
    percent = 100 - percent;
  return percent;
}

void Ramp::rampTo(int percent) //uses the last ramp speed.
{
  if (percentToPWM(percent) == targetDutyCycle)
    return;
  targetDutyCycle = percentToPWM(percent);

  millisLastChange = millis();
  if ((targetDutyCycle - currentDutyCycle) < 0)
    changePerInterval = -changePerInterval;
}

void Ramp::rampTo(int percent, long time) // go to percent in given time.
{
  targetDutyCycle = percentToPWM(percent);
  int delta = abs(targetDutyCycle - currentDutyCycle);
  calcRampSpeed(delta, time);

  millisLastChange = millis();
}

void Ramp::update()
{
  if (currentDutyCycle == targetDutyCycle)
  {
    isRamping = false;
    return;
  }
  else
    isRamping = true;

  if ((targetDutyCycle - currentDutyCycle) < 0)
    changePerInterval = -changePerInterval;

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
    millisLastChange = currentMillis;
  }
}

void Ramp::calcRampSpeed(int dutyCycle, long time)
{
  if (dutyCycle > time)
  {
    interval = 1;
    changePerInterval = dutyCycle / time;
  }
  else if (dutyCycle < time)
  {
    changePerInterval = 1;
    interval = time / dutyCycle;
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