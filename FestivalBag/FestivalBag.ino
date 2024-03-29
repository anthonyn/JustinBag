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
#define circlePin    5
//Mid Jem
#define star1Pin    6
//Top Jem
#define star2Pin    7

#define COLOR_ORDER GRB
#define CHIPSET     WS2812 // WS2811 LPD8806

#define numStripLeds    36
#define numStarLeds    7
#define numCircleLeds    16

#define BRIGHTNESS  40  // reduce power consumption
#define LED_DITHER  255  // try 0 to disable flickering
#define CORRECTION  TypicalLEDStrip

CRGB stripLeds[numStripLeds]; // Define the array of leds
CRGB circleLeds[numCircleLeds];
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

//int rightLeds[ledSectionLength] = {9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
//int bottomLeds[ledSectionLength] = {10, 11, 12, 13, 14, 15, 16, 17, 18, 19};
//int leftLeds[ledSectionLength] = {20, 21, 22, 23, 24, 25, 26, 27, 28, 29};

int rightLeds[18] = {17, 16, 15, 14, 13, 12, 11, 10,  9,  8,  7,  6,  5,  4,  3,  2,  1,  0};
int leftLeds[18] =  {18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35};

//dot stuff
int redDotLocation = 0;
long redDotFadeTimer = 0;
long redDotStayTime = 100;
long redDotStayTimer = 0;
int redDotFadeCounter = 0;


void setup() {
  // FastLED setup
  FastLED.addLeds<CHIPSET, stripPin, COLOR_ORDER>(stripLeds, numStripLeds).setCorrection(CORRECTION);
  FastLED.addLeds<CHIPSET, circlePin, COLOR_ORDER>(circleLeds, numCircleLeds).setCorrection(CORRECTION);
  FastLED.addLeds<CHIPSET, star1Pin, COLOR_ORDER>(star1Leds, numStarLeds).setCorrection(CORRECTION);
  FastLED.addLeds<CHIPSET, star2Pin, COLOR_ORDER>(star2Leds, numStarLeds).setCorrection(CORRECTION);

  FastLED.setBrightness( BRIGHTNESS );
  FastLED.setDither(LED_DITHER);
  FastLED.show(); // needed to reset leds to zero

  // This will set the IC ready for reading
  MSGEQ7.begin();

  Serial.begin(115200);
}
uint8_t gHue = 0; // rotating "base color" used by many of the patterns

void loop() {
  // Analyze without delay
  bool newReading = readMsgeq7();

  if (newReading) {
    soundReactiveOne();
    //updateBottomRow();
    //updateSides();
    FastLED.show();
  }

  //  EVERY_N_MILLISECONDS( 20 ) {
  //    gHue++;  // slowly cycle the "base color" through the rainbow
  //  }
  //  rainbow();
  //  FastLED.show();
}


bool readMsgeq7() {
  bool newReading = MSGEQ7.read(MSGEQ7_INTERVAL);
  return newReading;
}

int cval = 0;

void soundReactiveOne() {
  uint8_t valLow = MSGEQ7.get(MSGEQ7_LOW);
  uint8_t valMid = MSGEQ7.get(MSGEQ7_MID);
  uint8_t valHigh = MSGEQ7.get(MSGEQ7_HIGH);
  
  uint8_t valNewMid = MSGEQ7.get(MSGEQ7_2);
  
  // Reduce noise
  valLow = mapNoise(valLow);
  valMid = mapNoise(valMid);
  valHigh = mapNoise(valHigh);
  valNewMid = mapNoise(valNewMid);

  if (valLow > 10) {
    cval = (cval + 1) % 255;
  }


  //Stip animations
  //CRGB colorStripBottom = CRGB::White;
  CHSV hsv( cval, 255, 255); // pure blue in HSV Rainbow space
  CRGB rgb;
  hsv2rgb_rainbow( hsv, rgb);
  CRGB colorStripBottom = rgb;

  colorStripBottom.nscale8_video(rgb);

  //-->Bottom strip is bass
  //  for (int i = 0; i < ledSectionLength; i++) {
  //    stripLeds[bottomLeds[i]] = colorStripBottom;
  //  }

  Serial.print("Bass value is: ");
  Serial.println(valLow);

  //-->Side walls VU meter
  int positionOffset = 50;

  int sidePosition = map(valLow, 0, 235, 0, 14);

  Serial.print("side position is ");
  Serial.println(sidePosition);

  //paint lines
  for ( int i = 0; i < 18; i++) {
    if ( i < sidePosition) {
      stripLeds[leftLeds[i]] = rgb; //CRGB::White;
      stripLeds[rightLeds[i]] = rgb; //CRGB::White;
    }
    else {
      stripLeds[leftLeds[i]] = CRGB::Black;
      stripLeds[rightLeds[i]] = CRGB::Black;
    }
  }

  if (sidePosition > redDotLocation) {
    redDotLocation = sidePosition;
    redDotStayTimer = millis();
    redDotFadeCounter = 0;
    
  }
  else{
    if ((millis() - redDotStayTimer)  > redDotStayTime){
        if (redDotFadeCounter++ % 2 == 0){
          redDotLocation--;
          min(redDotLocation, 0);
        }
    }  
  }



  //paint reddot
  stripLeds[leftLeds[redDotLocation]] = CRGB::Red;
  stripLeds[rightLeds[redDotLocation]] = CRGB::Red;



////////
  //Mid Stars
  CRGB colorMid = CRGB::Red;
  colorMid.nscale8_video(valMid);

  //  //subtract from these value at some fixed rate in main loop  - make vars global
  if (valMid > 10) {
    gHue = gHue + 2;

    rainbow(circleLeds);
  }
  else {
    for ( int i = 0;  i < numCircleLeds; i++) {
      circleLeds[i] = CRGB::Black;
      
    }
  }

  CRGB colorNewMid = CRGB::Red;
  colorNewMid.nscale8_video(valNewMid);

  //  //subtract from these value at some fixed rate in main loop  - make vars global
  if (valNewMid > 10) {
    gHue = gHue + 2;

    rainbow(star1Leds);
  }
  else {
    for ( int i = 0;  i < numStarLeds; i++) {
      star1Leds[i] = CRGB::Black;
      
    }
  }


//  for ( int i = 0;  i < numStarLeds; i++) {
//    star1Leds[(numStarLeds - i) % 7] = circleLeds[i]*= 3;
//    // Serial.println(i);
//  }

  //  fill_solid(star0Leds, starCount, colorMid);
  //  fill_solid(star1Leds, starCount, colorMid);



  //High Stars
  CRGB colorHigh = CRGB::Blue;
  colorHigh.nscale8_video(valHigh);
  fill_solid(star2Leds, starCount, colorHigh);

}





//
//
//void updateBottomRow() {
//  uint8_t valLow = MSGEQ7.get(MSGEQ7_LOW);
//  uint8_t valMid = MSGEQ7.get(MSGEQ7_MID);
//  uint8_t valHigh = MSGEQ7.get(MSGEQ7_HIGH);
//  // Reduce noise
//  valLow = mapNoise(valLow);
//  valMid = mapNoise(valMid);
//  valHigh = mapNoise(valHigh);
//
//  // Visualize leds to the beat
//  CRGB colorLow = CRGB::White;
//  colorLow.nscale8_video(valLow);
//
//  CRGB colorMid = CRGB::Red;
//  colorMid.nscale8_video(valMid);
//  CRGB colorHigh = CRGB::Blue;
//  colorHigh.nscale8_video(valHigh);
//
//  //star0Leds
//  //fill_solid(leds, NUM_LEDS, color);
//  //fill_solid(star0Leds, starCount, CRGB::White);
//
//  fill_solid(star0Leds, starCount, colorMid);
//  fill_solid(star1Leds, starCount, colorMid);
//  fill_solid(star2Leds, starCount, colorHigh);
//
//  fill_solid(stripLeds, numStripLeds, colorLow);
//
//
//  //  for (int i = 0; i < ledSectionLength; i++) {
//  //    stripLeds[bottomLeds[i]] = colorLow;
//  //  }
//
//
//}
//
//void updateSides() {
//  uint8_t val = MSGEQ7.get(MSGEQ7_MID);
//  //uint8_t val = MSGEQ7.get(MSGEQ7_HIGH);
//  //val = mapNoise(val);
//  //Serial.println(val);
//  val = map(val, 0, 255, 0, 10);
//  for ( int i = 0; i < ledSectionLength; i++) {
//    if ( i > val) {
//      stripLeds[leftLeds[i]] = CRGB::Red;
//      stripLeds[rightLeds[i]] = CRGB::Red;
//    }
//    else {
//      stripLeds[leftLeds[i]] = CRGB::Black;
//      stripLeds[rightLeds[i]] = CRGB::Black;
//    }
//  }
//}
//
//
