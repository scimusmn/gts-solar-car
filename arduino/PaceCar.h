// Creates pace car on neopixel object
#ifndef PaceCar_h
#define PaceCar_h

#include "Arduino.h"
#include <Adafruit_NeoPixel.h>

class PaceCar
{
private:
    unsigned long startMillis, lastUpdate;
    int frontPixelIndex;
    Adafruit_NeoPixel *pixels;
    void lightCar(int);
    int moveInterval; //millis

public:
    PaceCar(Adafruit_NeoPixel *);
    bool hasWon;
    void startingLine();
    void start(long);
    void update();
};

PaceCar::PaceCar(Adafruit_NeoPixel *_pixels)
{
    this->pixels = _pixels;
}

void PaceCar::update()
{
    if (frontPixelIndex == (pixels->numPixels()))
    {
        hasWon = true;
        return;
    }

    unsigned long currentMillis = millis();
    if ((currentMillis - lastUpdate) > moveInterval)
    {
        frontPixelIndex++;
        lightCar(frontPixelIndex);
    }
}

void PaceCar::startingLine()
{
    hasWon = false;
    frontPixelIndex = 0;
    lightCar(frontPixelIndex);
}

void PaceCar::start(long raceTime)
{
    moveInterval = raceTime / (pixels->numPixels());
}

void PaceCar::lightCar(int pixelIndex)
{
    for (int i = 0; i < 4; i++) // turn on pixelIndex and the 3 pixels behind it.
    {
        pixelIndex--;
        if ((pixelIndex) < 0)
            pixelIndex = pixels->numPixels() - 1;
        pixels->setPixelColor(pixelIndex, 139, 0, 139);
    }
    pixels->setPixelColor(pixelIndex - 1, 0, 0, 0); //turn off pixel right behind new pace car.
    pixels->show();
}

#endif
