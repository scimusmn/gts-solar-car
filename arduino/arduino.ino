// Written by Joe Meyer, Fabricator III at the Science Museum of Minnesota in St. Paul
// For Gateway To Science, Bismark, ND
// Component: Solar Car 1004

#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <Arduino.h>
#include "Ramp.h"
#include "Encoder.h"
#include "PaceCar.h"
#include "Button.h"

const int LEDRingPIN = 7; // neopixel pin
const int NUM_LEDS = 48;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LEDRingPIN, NEO_RGB + NEO_KHZ800);

Ramp halogen(false, 3);
Ramp motor(true, 10);
Encoder car(11, 6, 30);
PaceCar paceCar(&strip);

const int HardStartBtn = 4;
const int HardBtnLED = 5;
const int MedStartBtn = 8;
const int MedBtnLED = 9;
const int EasyStartBtn = 12;
const int EasyBtnLED = 13;
const int EasyTimePot = A0; // potentiometer used to set easy race time for win.
const int HardTimePot = A2; // potentiometer used to set had race time for win.

Button easyBtn;
Button medBtn;
Button hardBtn;

bool isWaiting;
long timeToRace;

void setup()
{
  pinMode(EasyTimePot, INPUT);
  pinMode(HardTimePot, INPUT);
  motor.setFullRampSpeed(1000);

  easyBtn.setup(EasyStartBtn, EasyBtnLED, [](int state) {
    if (state)
    {
      timeToRace = analogRead(EasyTimePot);
      timeToRace = map(timeToRace, 0, 1023, 30000, 90000); //convert to milliseconds for race time
      medBtn.lightOff();
      hardBtn.lightOff();
      isWaiting = false;
    }
  });

  medBtn.setup(MedStartBtn, MedBtnLED, [](int state) {
    if (state)
    { //medium time is half way between the easy and hard time.
      timeToRace = analogRead(HardTimePot);
      long hardTime = map(timeToRace, 0, 1023, 5000, 30000); //convert to milliseconds for race time
      timeToRace = analogRead(EasyTimePot);
      long easyTime = map(timeToRace, 0, 1023, 30000, 90000); //convert to milliseconds for race time
      timeToRace = (easyTime + hardTime) / 2;
      easyBtn.lightOff();
      hardBtn.lightOff();
      isWaiting = false;
    }
  });

  hardBtn.setup(HardStartBtn, HardBtnLED, [](int state) {
    if (state)
    {
      timeToRace = analogRead(HardTimePot);
      timeToRace = map(timeToRace, 0, 1023, 5000, 30000); //convert to milliseconds for race time
      medBtn.lightOff();
      easyBtn.lightOff();
      isWaiting = false;
    }
  });

  //neo pixels
  strip.begin();
  allLEDS(0, 0, 255); //all leds to blue, show life!
  delay(1000);
  allLEDS(0, 0, 0); // turn leds off
}

void loop()
{
  easyBtn.lightOn();
  medBtn.lightOn();
  hardBtn.lightOn();

  isWaiting = true;
  while (isWaiting)
  {
    easyBtn.update();
    medBtn.update();
    hardBtn.update();
    car.update();
  }
  startRace();
}

void startRace()
{
  halogen.rampTo(40, 1000);   // warm lamp up.
  colorWipe(40, 255, 0, 0);   //red
  colorWipe(40, 255, 255, 0); //yellow
  colorWipe(40, 0, 255, 0);   //green
  allLEDS(0, 0, 0);           // turn leds off
  halogen.setPercent(100);
  paceCar.start(timeToRace);
  bool isRacing = true;
  while (isRacing)
  {
    paceCar.update();
    car.update();

    if (paceCar.hasWon) //visitor lost;
    {
      isRacing = false;
    }
    if (car.leftFinish && (car.getPositionDeg() == 0))
    {
      isRacing = false;
    }
  }
  halogen.setPercent(4);
  resetCar();
}

void resetCar()
{
  bool isResetting = true;
  int carAtStartCounter = 0;
  if (paceCar.hasWon)
    allLEDS(255, 0, 0); //red
  else
    allLEDS(0, 255, 0); //green

  while (isResetting)
  {
    motor.update();
    car.update();

    int positionDeg = car.getPositionDeg();
    if ((positionDeg > 250) && (positionDeg < 330))
    {
      int t = 100 - (positionDeg - 250);
      motor.rampTo(t);
    }
    if (positionDeg > 330)
      motor.rampTo(15);
    if ((positionDeg > 0) && (positionDeg < 250))
    {
      carAtStartCounter = 0;
      motor.rampTo(100);
    }

    if (car.getPositionDeg() == 0)
    {
      motor.setPercent(0);
      //kind of a hacky way to tell if the car has sat at start for a bit or wizzed by.
      carAtStartCounter++;
      if (carAtStartCounter > 200)
        isResetting == false;
    }
  }

  allLEDS(0, 0, 0); // turn leds off
  paceCar.startingLine();

  // TODO blink lights during reset according to win or lose.
  // if (isResetting)
  // {
  //   int j = 0;
  //   if (!paceCar.hasWon)
  //   { // create rainbow pattern.
  //     for (int i = 0; i < strip.numPixels(); i++)
  //     {
  //       strip.setPixelColor(i, Wheel((i + j) & 255));
  //     }
  //     strip.show();
  //     j++;
  //   }
  //   else
  //   { // lose lights, blinking red, ledState toggles off/on
  //     if (currentMillis - previousBlinkMillis >= 500)
  //     {
  //       previousBlinkMillis = currentMillis;
  //       if (ledState == 0)
  //       {
  //         allLEDS(255, 0, 0);
  //         ledState = 1;
  //       }
  //       else
  //       {
  //         allLEDS(0, 0, 0);
  //         ledState = 0;
  //       }
  //     }
  //   }
  // }
}

void colorWipe(int wait, int R, int G, int B)
{
  for (uint16_t i = 0; i < 15; i++)
  {
    strip.setPixelColor(15 - i, R, G, B); //color wipe
    delay(wait);
    strip.show();
  }
}

void allLEDS(int R, int G, int B)
{ //turn off all LEDs
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
