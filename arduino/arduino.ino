// Written by Joe Meyer, Fabricator III at the Science Museum of Minnesota in St. Paul
// For Gateway To Science, Bismark, ND
// Component: Solar Car 1004

#include <Adafruit_NeoPixel.h>
#include <Wire.h>

const int LightRelayPIN = 3;
const int HardStartBtn = 4;
const int HardBtnLED = 5;
const int HallLinePIN = 6;
const int LEDRingPIN = 7;
const int MedStartBtn = 8;
const int MedBtnLED = 9;
const int MotorPIN = 10;
const int EncoderPIN = 11;
const int EasyStartBtn = 12;
const int EasyBtnLED = 13;
const int EasyTimePot = A0;
const int HardTimePot = A2;

const int NUM_LEDS = 48;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LEDRingPIN, NEO_RGB + NEO_KHZ800);

int lapsToWin = 1;       // The number of laps to be completed for win.
long timeToRace = 20000; // in millisec, time you have to complete the designated # of laps.

int resetting = 1, racing = 0, hasWon = 1, ledState = 0, encoderLastRead = 0; // flags
int lapCounter, currentSpeed = 0, targetSpeed, position;
int timingIndexLED;
unsigned long startMillis = 0, previousTimingMillis, previousBlinkMillis, previousRampMillis, currentMillis = 0;
long timeElapsed = 0;
int hallLineState = 0, hallLinePrevState; // variables for reading and storing the hall sensors status hallBeforeState, hallBeforePrevState,

void setup()
{
  //neo pixels
  strip.begin();
  allLEDS(0, 0, 255); //all leds to blue, show life!

  pinMode(HallLinePIN, INPUT_PULLUP);
  pinMode(LightRelayPIN, OUTPUT);
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

  Serial.begin(9600);
  analogWrite(MotorPIN, 0);
  delay(1000);
  allLEDS(0, 0, 0); // turn leds off
}

void loop()
{
  currentMillis = millis();
  timeElapsed = currentMillis - startMillis;

  //increments position from change in encoder beam break.
  int encoderRead = digitalRead(EncoderPIN);
  if (encoderLastRead != encoderRead)
  {
    position++;
    encoderLastRead = encoderRead;
  }

  // blink lights during reset according to win or lose.
  if (resetting)
  {
    if (hasWon)
    { // create rainbow pattern. ledState used to change color wheel.
      for (int i = 0; i < strip.numPixels(); i++)
      {
        strip.setPixelColor(i, Wheel((i + ledState) & 255));
      }
      strip.show();
      ledState++;
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
  //read new sensor data
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
    targetSpeed = 255;
    Serial.print("ERROR");
    position = 5;
  }

  // edge detection
  if (hallLineState && !hallLinePrevState)
  { // passed sensor hallLine
    targetSpeed = 255;
    Serial.println("pass LINE");
    position = 0;
  }
  if (!hallLineState && hallLinePrevState)
  { // On sensor hallLine
    Serial.println("ON LINE");
    position = 0;
    currentSpeed = 0;
    targetSpeed = 0;
    if (racing == 1)
    {
      lapCounter++;
    }
  }

  if (currentMillis - previousRampMillis >= 5)
  { // every 5 ms check if speed should increase
    previousRampMillis = currentMillis;
    if (currentSpeed < targetSpeed)
    { // increase speed by 1 if below target.
      currentSpeed = currentSpeed + 1;
    }
  }

  if (!hallLineState && !hallLinePrevState)
  { // still on sensor hallLine
    if (resetting && timeElapsed > 4000)
    {
      Serial.println("Done Resetting");
      resetting = 0;
      allLEDS(0, 0, 0);
      strip.setPixelColor(45, 139, 0, 139); // set color purple to create virtual pace car
      strip.setPixelColor(46, 139, 0, 139); //
      strip.setPixelColor(47, 139, 0, 139); //
      strip.setPixelColor(0, 139, 0, 139);  //  Start line is at LED 0.
      strip.show();
    }
  }

  if (resetting == 1 && timeElapsed > 2000)
  { // 2 sec delay after Solar lamp is off, then kick in motor reset.
    analogWrite(MotorPIN, currentSpeed);
  }
  else
  {
    analogWrite(MotorPIN, 0);
    currentSpeed = 0;
  }

  if (resetting == 0 && racing == 0)
  { // ready to race, waiting button press

    waitToStart();
  }

  if (racing == 1)
  {
    // change LED timing ring
    if (currentMillis - previousTimingMillis >= timeToRace / (strip.numPixels() * lapsToWin))
    {
      previousTimingMillis = currentMillis;

      strip.setPixelColor(timingIndexLED, 139, 0, 139); // turn front LED on.
      int timingIndexLEDoff = timingIndexLED - 4;
      if (timingIndexLEDoff < 0)
      {
        timingIndexLEDoff = timingIndexLEDoff + 48;
      }
      strip.setPixelColor(timingIndexLEDoff, 0, 0, 0); // turn back LED off.
      strip.show();
      timingIndexLED++;
      if (timingIndexLED == 48)
      {
        timingIndexLED = 0;
      }
    }

    if (lapCounter == lapsToWin)
    {
      Serial.write(timeElapsed);
      endRace(1);
    }

    if (timeElapsed > timeToRace)
    {
      endRace(0);
    }
  }
}

void waitToStart()
{ // wait for user to press start button.
  Serial.println("Waiting for button press");
  digitalWrite(EasyBtnLED, HIGH); //light Start button telling visitor they can play!
  digitalWrite(MedBtnLED, HIGH);  //light Start button telling visitor they can play!
  digitalWrite(HardBtnLED, HIGH); //light Start button telling visitor they can play!
  bool is_waiting = true;
  long easyTime = analogRead(EasyTimePot);
  easyTime = map(easyTime, 0, 1023, 30000, 90000); //convert to milliseconds for race time (10 sec thru 61.15 sec)
  long hardTime = analogRead(HardTimePot);
  hardTime = map(hardTime, 0, 1023, 5000, 30000); //convert to milliseconds for race time (10 sec thru 61.15 sec)

  while (is_waiting)
  {
    if (digitalRead(EasyStartBtn) == LOW)
    {
      timeToRace = easyTime;
      is_waiting = false;
      Serial.print("easy time: ");
    }
    if (digitalRead(HardStartBtn) == LOW)
    {
      timeToRace = hardTime;
      is_waiting = false;
      Serial.print("hard time: ");
    }
    if (digitalRead(MedStartBtn) == LOW)
    {
      timeToRace = (easyTime + hardTime) / 2;
      is_waiting = false;
      Serial.print("med time: ");
    }
  }

  racing = 1;
  timingIndexLED = 0; // LED 11 is starting line.
  resetting = 0;
  timeElapsed = 0;
  currentSpeed = 0;
  Serial.println(timeToRace / 1000);

  digitalWrite(EasyBtnLED, LOW);
  digitalWrite(MedBtnLED, LOW);
  digitalWrite(HardBtnLED, LOW);
  colorWipe(40, 255, 0, 0);   //red
  colorWipe(40, 255, 255, 0); //yellow
  colorWipe(40, 0, 255, 0);   //green
  for (int i = 1; i < 44; i++)
  {
    strip.setPixelColor(i, 0, 0, 0); // turn off starting light LEDS
  }
  strip.show();
  analogWrite(LightRelayPIN, 255);               // turn the lamp full on.
  previousTimingMillis = startMillis = millis(); //record start time, initialize perviousTimingMillis for new race.
  lapCounter = 0;
}

void endRace(int hw)
{
  hasWon = hw;
  analogWrite(LightRelayPIN, 5); // lamp is "off" at 5/255 (about 2%) power to keep the filament warm and increase lamp life.
  racing = 0;
  resetting = 1;
  ledState = 0;
  startMillis = millis();
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
