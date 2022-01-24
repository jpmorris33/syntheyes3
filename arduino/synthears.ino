//
//  Synth Ears for Arduino (SynthEyes without the eyes, basically)
// 
//  V1.0.0 - Drive status lights including red fault mode with voice control
//  V1.1.0 - Hold fault mode for as long as FAULT_PIN is low instead of timing it, improve brightness
//

/*
BSD 3-Clause License

Copyright (c) 2022, Joseph P Morris, IT-HE SOFTWARE
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

//
//  Attach the status lights to the following pins (configurable), these are
//  adjacent to each other on the Nano's 6-pin header
//
//  5V
//  DIN - pin 11
//  GND
//
//  In addition, the microphone ADC is on pin A7
//  Pin 2 can be used to trigger the red FAULT mode
//

#define VOICE_DETECTOR    // Flash the status lights if a microphone detects something

#include <FastLED.h>

// Configurables, adjust to taste

// RGB triplets for the status light colour, default to yellow (full red, half green, no blue)
#define COLOUR_RED   0xff
#define COLOUR_GREEN 0x80
#define COLOUR_BLUE  0x00

// If the FAULT_PIN is set low, make it this colour instead
#define FAULT_RED 0x0ff
#define FAULT_GREEN 0x00
#define FAULT_BLUE 0x00

#define STATUSBRIGHT 255
#define VOICEBRIGHT 10

#define STATUS_PIN 11    // Neopixels DIN pin for status LEDs
#define VOICE_PIN A7    // Audio input for flashing the status pins
#define FAULT_PIN 2

#define ADC_THRESHOLD 128 // Voice activation threshold



#define FRAME_IN_MS 20  // Delay per animation frame in milliseconds (20 default)
#define WAIT_IN_MS  60  // Delay per tick in milliseconds when waiting to blink again (60 default)
#define MIN_DELAY    5   // Minimum delay between blinks
#define MAX_DELAY    250 // Maximum delay between blinks
#define STATUS_DIVIDER 32  // This controls the speed of the status light chaser, bigger is slower (def 32)

#define PIXELS 128
#define PIXELS_PER_PANEL 64
#define STATUSPIXELS 6

#define STEPS (STATUSPIXELS*2)

// Functions
void wait(int ms);
void statusCycle(unsigned char r, unsigned char g, unsigned char b);

// Status light variables
CRGB statusbuffer[STATUSPIXELS];
CRGB colour(COLOUR_RED,COLOUR_GREEN,COLOUR_BLUE);
CLEDController *statusController;
unsigned char ramp[STEPS];


//
//  Here's the actual implementation
//

void setup() {
  pinMode(FAULT_PIN,INPUT_PULLUP);
  
  // Initialise the status lights
  statusController = &FastLED.addLeds<NEOPIXEL, STATUS_PIN>(statusbuffer,STATUSPIXELS);
  for(int ctr=0;ctr<STATUSPIXELS;ctr++) {
    statusbuffer[ctr]=CRGB::Red;
  }
  statusController->showLeds(STATUSBRIGHT);

  // Build lookup table for pulsating status lights
  // I'm sure there's a smarter way to do this on the fly, but...
  int ramping=0;
  for(int ctr=0;ctr<STEPS;ctr++) {
    ramp[ctr]=ramping;
    if(ctr<STEPS/2) {
      ramping+=(256/(STEPS*2));
    } else {
      ramping-=(256/(STEPS*2));
    }
  }

  statusCycle(COLOUR_RED,COLOUR_GREEN,COLOUR_BLUE);
}

void loop() {
    wait(1000);
}

//
//  Wait, and cue up a reaction if we detect one via GPIO
//

void wait(int ms) {
  for(int ctr=0;ctr<ms;ctr++) {
    delay(1);
    statusCycle(COLOUR_RED,COLOUR_GREEN,COLOUR_BLUE);
  }
}

static int maxpos = 255 - (255%STEPS);

void statusCycle(unsigned char r, unsigned char g, unsigned char b) {
  uint16_t ctr;
  static uint16_t pos=0;
  static int divider=0;
  int maxdiv=STATUS_DIVIDER;
  static int brightcount=0;
  int val;
  #ifdef VOICE_DETECTOR
    bool bright = !(analogRead(VOICE_PIN) > ADC_THRESHOLD);
  #else
    bool bright = false;
  #endif
  bool faultState = !digitalRead(FAULT_PIN);

  if(bright) {
    brightcount=2;
  }

  divider++;
  if(divider > maxdiv) {
    pos++;
    divider=0;
    if(brightcount > 0) {
      brightcount--;
    }
  }
  if(pos > maxpos) {
    pos=0;
  }

  for(ctr=0; ctr< STATUSPIXELS; ctr++) {
    // If they're in an emergency state, make the lights red
    if(faultState) {
      statusbuffer[ctr] = CRGB(FAULT_RED,FAULT_GREEN,FAULT_BLUE);
    } else {
      statusbuffer[ctr] = CRGB(r,g,b);
    }
    if(brightcount > 0) {
      statusbuffer[ctr].nscale8(255);
    } else {
      if(faultState) {
        // All blink together
        val = ramp[pos%STEPS] * 2;
        if(val > 255) val=255;
        statusbuffer[ctr].nscale8(val);
      }
      else {
        // Fairground effect
        val = ramp[(ctr+pos)%STEPS] * 2;
        if(val > 255) val=255;
        statusbuffer[ctr].nscale8(val);
      }
    }
  }

  statusController->showLeds(brightcount > 0?VOICEBRIGHT:STATUSBRIGHT);
}
