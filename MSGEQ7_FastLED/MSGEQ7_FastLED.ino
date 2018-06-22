/*
  Copyright (c) 2014-2016 NicoHood
  See the readme for credit to other people.

  MSGEQ7 FastLED example
  Output via Led strip and FastLED library

  Reads MSGEQ7 IC with 7 different frequencies from range 0-255
  63Hz, 160Hz, 400Hz, 1kHz, 2.5kHz, 6.25KHz, 16kHz
*/

// FastLED
#include "FastLED.h"

#define stripPin    4
//Mid Jem
#define star0Pin    5
//Mid Jem
#define star1Pin    6
//Top Jem
#define star2Pin    7

#define COLOR_ORDER GRB
#define CHIPSET     WS2812 // WS2811 LPD8806

#define numStripLeds    30
#define numStarLeds    7

#define BRIGHTNESS  255  // reduce power consumption
#define LED_DITHER  255  // try 0 to disable flickering
#define CORRECTION  TypicalLEDStrip

CRGB stripLeds[numStripLeds]; // Define the array of leds
CRGB star0Leds[numStarLeds];
CRGB star1Leds[numStarLeds];
CRGB star2Leds[numStarLeds];



// MSGEQ7
#include "MSGEQ7.h"
#define inputPin A0
#define pinReset A5
#define pinStrobe 2

#define MSGEQ7_INTERVAL ReadsPerSecond(50)
#define MSGEQ7_SMOOTH true

CMSGEQ7<MSGEQ7_SMOOTH, pinReset, pinStrobe, inputPin> MSGEQ7;

const int ledSectionLength = 10;
const int starCount = 7;

//all in viewers perspecrtivwe
byte rightLeds[ledSectionLength] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
byte bottomLeds[ledSectionLength] = {10, 11, 12, 13, 14, 15, 16, 17, 18, 19};
byte leftLeds[ledSectionLength] = {29, 28, 27, 26, 25, 24, 23, 22, 21, 20};




void setup() {
  // FastLED setup
  FastLED.addLeds<CHIPSET, stripPin, COLOR_ORDER>(stripLeds, numStripLeds).setCorrection(CORRECTION);
  FastLED.addLeds<CHIPSET, star0Pin, COLOR_ORDER>(star0Leds, numStarLeds).setCorrection(CORRECTION);
  FastLED.addLeds<CHIPSET, star1Pin, COLOR_ORDER>(star1Leds, numStarLeds).setCorrection(CORRECTION);
  FastLED.addLeds<CHIPSET, star2Pin, COLOR_ORDER>(star2Leds, numStarLeds).setCorrection(CORRECTION);




  FastLED.setBrightness( BRIGHTNESS );
  FastLED.setDither(LED_DITHER);
  FastLED.show(); // needed to reset leds to zero

  // This will set the IC ready for reading
  MSGEQ7.begin();

  Serial.begin(57600);
}

void loop() {
  // Analyze without delay
  bool newReading = readMsgeq7();

  if (newReading) {
    updateBottomRow();
    //updateSides();
    FastLED.show();
  }
}


bool readMsgeq7() {
  bool newReading = MSGEQ7.read(MSGEQ7_INTERVAL);
  return newReading;
}

void updateBottomRow() {
  uint8_t valLow = MSGEQ7.get(MSGEQ7_LOW);
  uint8_t valMid = MSGEQ7.get(MSGEQ7_MID);
  uint8_t valHigh = MSGEQ7.get(MSGEQ7_HIGH);
  // Reduce noise
  valLow = mapNoise(valLow);
  valMid = mapNoise(valMid);
  valHigh = mapNoise(valHigh);

  // Visualize leds to the beat
  CRGB colorLow = CRGB::White;
  colorLow.nscale8_video(valLow);
  CRGB colorMid = CRGB::Red;
  colorMid.nscale8_video(valMid);
  CRGB colorHigh = CRGB::Blue;
  colorHigh.nscale8_video(valHigh);
  //star0Leds
  //fill_solid(leds, NUM_LEDS, color);
  //fill_solid(star0Leds, starCount, CRGB::White);

  fill_solid(star0Leds, starCount, colorMid);
  fill_solid(star1Leds, starCount, colorMid);
  fill_solid(star2Leds, starCount, colorHigh);
  fill_solid(stripLeds, numStripLeds, colorLow);
  //
  //  for (int i = 0; i < ledSectionLength; i++) {
  //    stripLeds[bottomLeds[i]] = color;
  //  }
}

void updateSides() {
  uint8_t val = MSGEQ7.get(MSGEQ7_MID);
  //uint8_t val = MSGEQ7.get(MSGEQ7_HIGH);
  //val = mapNoise(val);
  //Serial.println(val);
  val = map(val, 0, 255, 0, 10);
  for ( int i = 0; i < ledSectionLength; i++) {
    if ( i > val) {
      stripLeds[leftLeds[i]] = CRGB::Red;
      stripLeds[rightLeds[i]] = CRGB::Red;
    }
    else {
      stripLeds[leftLeds[i]] = CRGB::Black;
      stripLeds[rightLeds[i]] = CRGB::Black;
    }
  }
}



