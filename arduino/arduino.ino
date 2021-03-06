// Written by Joe Meyer, Fabricator III at the Science Museum of Minnesota in
// St. Paul For Gateway To Science, Bismark, ND Component: Solar Car 1004

#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include <Wire.h>

#include "Ramp.h"

Ramp halogen(true, 3); // pwm via Mosfet of halogen lamp

const int HardStartBtn = 4;
const int HardBtnLED = 5;
const int HallLinePIN = 6; // hall sensor input on starting/finish line
const int LEDRingPIN = 7;  // neopixel pin
const int MedStartBtn = 8;
const int MedBtnLED = 9;
const int MotorPIN = 10;   // pwm signal on this pin drives motor speed
const int EncoderPIN = 11; // signal from encoder determines car position.
const int EasyStartBtn = 12;
const int EasyBtnLED = 13;
const int EasyTimePot = A0; // potentiometer used to set easy race time for win.
const int HardTimePot = A2; // potentiometer used to set had race time for win.

const int NUM_LEDS = 48;

// pwm value (0-255) of power for motor to assist during the race.
// a value of 30 caused the car to drive without any solar power.
int motorAssist = 22;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LEDRingPIN, NEO_RGB + NEO_KHZ800);
int rainbow = 0;
long timeToRace = 20000; // in millisec, time you have to complete a lap.

bool isResetting = 1, isRacing = 0, hasWon = 0, ledState = 0, encoderLastRead = 0;
int currentSpeed = 0, targetSpeed, position = 70;
int paceCarFirstLED; // index number for neopixel strip to create 4 pixel pace car.
unsigned long startMillis = 0, previousTimingMillis, previousBlinkMillis,
              previousRampMillis, currentMillis = 0;
long timeElapsed = 0;
int hallLineState,
    hallLinePrevState; // variables for edge detection of the hall sensor

void setup()
{
  // neo pixels
  strip.begin();
  allLEDS(0, 0, 255); // all leds to blue, show life!
  delay(1000);

  pinMode(HallLinePIN, INPUT_PULLUP);
  pinMode(HardStartBtn, INPUT);
  pinMode(HardBtnLED, OUTPUT);
  pinMode(MedStartBtn, INPUT);
  pinMode(MedBtnLED, OUTPUT);
  pinMode(EasyStartBtn, INPUT);
  pinMode(EasyBtnLED, OUTPUT);
  pinMode(MotorPIN, OUTPUT);
  pinMode(EncoderPIN, INPUT);
  pinMode(EasyTimePot, INPUT);
  pinMode(HardTimePot, INPUT);
  startMillis = millis();

  hallLineState = digitalRead(HallLinePIN);
  analogWrite(MotorPIN, 0);
  allLEDS(0, 0, 0); // turn leds off
}

void loop()
{
  currentMillis = millis();
  timeElapsed = currentMillis - startMillis;
  halogen.update();

  // increments position from change in encoder beam break.
  int encoderRead = digitalRead(EncoderPIN);
  if (encoderLastRead != encoderRead)
  {
    position++;
    encoderLastRead = encoderRead;
  }

  // blink lights during reset according to win or lose.
  if (isResetting)
  {
    if (hasWon)
    { // create rainbow pattern.
      for (int i = 0; i < strip.numPixels(); i++)
      {
        strip.setPixelColor(i, Wheel((i + rainbow) & 255));
      }
      strip.show();
      rainbow++;
    }
    else
    { // lose lights, blinking red, ledState toggles off/on
      if (currentMillis - previousBlinkMillis >= 500)
      {
        previousBlinkMillis = currentMillis;
        if (ledState == 0)
        {
          allLEDS(255, 0, 0);
          ledState = 1;
        }
        else
        {
          allLEDS(0, 0, 0);
          ledState = 0;
        }
      }
    }
  }

  hallLinePrevState = hallLineState;
  // read new sensor data
  hallLineState = digitalRead(HallLinePIN);

  if ((position > 45) && (position < 56))
  {
    targetSpeed = 255 - (position - 45) * 20;
    if (currentSpeed > targetSpeed)
      currentSpeed = targetSpeed;
  }
  if (position > 55)
  {
    targetSpeed = 40;
    if (currentSpeed > targetSpeed)
      currentSpeed = targetSpeed;
  }
  if (position > 65)
  {
    targetSpeed = 50;
  }

  // edge detection
  if (hallLineState && !hallLinePrevState)
  { // passed sensor hallLine
    targetSpeed = 255;
    position = 0;
  }
  if (!hallLineState && hallLinePrevState)
  { // On sensor hallLine
    position = 0;
    currentSpeed = 0;
    targetSpeed = 0;
    if (isRacing == 1)
    {
      endRace(1);
    }
  }

  if (!hallLineState && !hallLinePrevState) // still on sensor hallLine
  {
    if (isResetting && timeElapsed > 4000)
    {
      isResetting = 0;
      analogWrite(MotorPIN, 0);
      currentSpeed = 0;
      allLEDS(0, 0, 0);
      strip.setPixelColor(45, 139, 0, 139); // set color purple to create virtual pace car
      strip.setPixelColor(46, 139, 0, 139); //
      strip.setPixelColor(47, 139, 0, 139); //
      strip.setPixelColor(0, 139, 0, 139);  //  Start line is at LED 0.
      strip.show();
    }
  }

  // 2 sec delay after Solar lamp is off, then kick in motor reset.
  if (isResetting == 1 && timeElapsed > 2000)
  {
    analogWrite(MotorPIN, currentSpeed);
    // every 5 ms check if speed should increase
    if (currentMillis - previousRampMillis >= 5)
    {
      previousRampMillis = currentMillis;
      if (currentSpeed < targetSpeed)
      { // increase speed by 1 if below target.
        currentSpeed = currentSpeed + 1;
      }
    }
  }

  // ready to race, waiting button press
  if (isResetting == 0 && isRacing == 0)
    waitToStart();

  if (isRacing == 1)
  {
    // change LED timing ring
    if ((currentMillis - previousTimingMillis) >= (timeToRace / strip.numPixels()))
    {
      previousTimingMillis = currentMillis;

      strip.setPixelColor(paceCarFirstLED, 139, 0, 139); // turn front LED on.
      int paceCarFirstLEDoff = paceCarFirstLED - 4;
      if (paceCarFirstLEDoff < 0)
      {
        paceCarFirstLEDoff = paceCarFirstLEDoff + 48;
      }
      strip.setPixelColor(paceCarFirstLEDoff, 0, 0, 0); // turn back LED off.
      strip.show();
      paceCarFirstLED++;
      if (paceCarFirstLED == 48)
        paceCarFirstLED = 0;
    }

    if (timeElapsed > timeToRace)
      endRace(0);
  }
}

void waitToStart() // wait for user to press start button.
{
  digitalWrite(EasyBtnLED, HIGH); // light Start button telling visitor they can play!
  digitalWrite(MedBtnLED, HIGH);  // light Start button telling visitor they can play!
  digitalWrite(HardBtnLED, HIGH); // light Start button telling visitor they can play!
  bool is_waiting = true;
  long easyTime = analogRead(EasyTimePot);
  easyTime = map(easyTime, 0, 1023, 30000, 90000); // convert to milliseconds for race time (10 sec thru 61.15 sec)
  long hardTime = analogRead(HardTimePot);
  hardTime = map(hardTime, 0, 1023, 5000, 30000); // convert to milliseconds for race time (10 sec thru 61.15 sec)
  unsigned long waitStartMillis;
  waitStartMillis = millis();
  bool lightOut = false;

  while (is_waiting)
  {
    currentMillis = millis();
    if ((currentMillis - waitStartMillis) > 30000 && !lightOut)
    {
      halogen.setPercent(0);
      lightOut = true;
    }

    if (digitalRead(EasyStartBtn) == LOW)
    {
      timeToRace = easyTime;
      is_waiting = false;
    }
    if (digitalRead(HardStartBtn) == LOW)
    {
      timeToRace = hardTime;
      is_waiting = false;
    }
    if (digitalRead(MedStartBtn) == LOW)
    {
      timeToRace = (easyTime + hardTime) / 2;
      is_waiting = false;
    }
  }

  isRacing = 1;
  paceCarFirstLED = 0;
  isResetting = 0;
  timeElapsed = 0;
  currentSpeed = 0;
  halogen.rampTo(30, 1000); // warm lamp up.
  digitalWrite(EasyBtnLED, LOW);
  digitalWrite(MedBtnLED, LOW);
  digitalWrite(HardBtnLED, LOW);
  colorWipe(40, 255, 0, 0);   // red
  colorWipe(40, 255, 255, 0); // yellow
  colorWipe(40, 0, 255, 0);   // green
  for (int i = 1; i < 44; i++)
  {
    strip.setPixelColor(i, 0, 0, 0); // turn off starting light LEDS
  }
  strip.show();
  // turn the lamp full on.
  halogen.rampTo(100, 100);
  analogWrite(MotorPIN, motorAssist); // Motor assist because halogen doesn't supply quite enough power.
  // record start time, initialize perviousTimingMillis for new race.
  previousTimingMillis = startMillis = millis();
}

void endRace(int hw)
{
  hasWon = hw;
  rainbow = 0;
  halogen.rampTo(4, 500); // fade lamp off quick.
  isRacing = 0;
  isResetting = 1;
  ledState = 0;
  startMillis = millis();
}

void colorWipe(int wait, int R, int G, int B)
{
  for (uint16_t i = 0; i < 15; i++)
  {
    strip.setPixelColor(15 - i, R, G, B); // color wipe
    delay(wait);
    halogen.update();
    strip.show();
  }
}

// turn all LEDs to a color (or off)
void allLEDS(int R, int G, int B)
{
  for (uint16_t i = 0; i < strip.numPixels(); i++)
  {
    strip.setPixelColor(i, R, G, B);
  }
  strip.show();
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos)
{
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85)
  {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3, 0);
  }
  if (WheelPos < 170)
  {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3, 0);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0, 0);
}
