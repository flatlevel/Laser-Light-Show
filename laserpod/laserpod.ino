/**
 * Pew motherfuckin' pew. 
 * 
 * @copyright 2015 Skyworks Aerial Systems.
 */
#ifdef __AVR__
  #include <avr/power.h>
#endif

#include <stdio.h>
#include <string.h>

#include <DmxSimple.h>
#include <Lasers.h> 

#include <FastLED.h>
#include <NeoPixelEffects.h>

#define INIT_NONE         

// Hardware pin mappings
#define LASER_DMX_PIN       9
#define LASER_ENABLE_PIN    3
#define DATA_PIN            A0
#define LED1                10
#define LED2                11
#define LED3                12
#define LED4                13

// General config
#define SERIAL_BAUDRATE     57600
#define NUM_LEDS            129
#define MAX_CMD_SIZE        64
#define MAX_BRIGHTNESS      0x33
#define UPDATE_PERIOD       50000

// Defines for different kinds of selections
#define FACE          0
#define RING1         1
#define RING2         2
#define RING3         3
#define RING4         4
#define ALL           5
#define HALFR         6
#define HALFL         7
#define DIAGR         8
#define DIAGL         9
#define NOHEAD        10
#define BACK          11
#define FRONT         12

// Ring / Face ranges
#define RING1_MIN     0
#define RING1_MAX     32
#define RING2_MIN     33
#define RING2_MAX     60
#define RING3_MIN     68
#define RING3_MAX     95
#define RING4_MIN     96
#define RING4_MAX     128
#define FACE_MIN      61
#define FACE_MAX      67

// Effect modules
CRGB leds[NUM_LEDS];
CRGB lowerLeds[4][3];
NeoPixelEffects effect[5]; //= NeoPixelEffects(leds, effectType, rangeStart, rangeEnd, areaOfEffect, delay_ms, grey, looping, dir);
Lasers lasers;

// Command parsing globals
char      cmd[MAX_CMD_SIZE] = { '\0' };
uint8_t   readchar = 0;
bool      readyforCmd = false;
uint8_t   loc = 0;

// Time related globals
uint32_t prevTime = 0, currTime = 0;


void InitEffects() {
#ifdef INIT_NONE
  effect[RING1] =   NeoPixelEffects(leds, NONE, RING1_MIN, RING1_MAX, 4, 15, 0, true, FORWARD);
  effect[RING2] =   NeoPixelEffects(leds, NONE, RING2_MIN, RING2_MAX, 4, 15, 0, true, FORWARD);
  effect[RING3] =   NeoPixelEffects(leds, NONE, RING3_MIN, RING3_MAX, 4, 15, 0, true, FORWARD);
  effect[RING4] =   NeoPixelEffects(leds, NONE, RING4_MIN, RING4_MAX, 4, 15, 0, true, FORWARD);
  effect[FACE] =    NeoPixelEffects(leds, NONE, FACE_MIN, FACE_MAX, 4, 15, 0, false, FORWARD);
#else
  effect[RING1] =   NeoPixelEffects(leds, FILLIN, RING1_MIN, RING1_MAX, 4, 15, 0x008EFF, true, FORWARD);
  effect[RING2] =   NeoPixelEffects(leds, FILLIN, RING2_MIN, RING2_MAX, 4, 15, 0x008EFF, true, FORWARD);
  effect[RING3] =   NeoPixelEffects(leds, FILLIN, RING3_MIN, RING3_MAX, 4, 15, 0x008EFF, true, FORWARD);
  effect[RING4] =   NeoPixelEffects(leds, FILLIN, RING4_MIN, RING4_MAX, 4, 15, 0x008EFF, true, FORWARD);
  effect[FACE] =    NeoPixelEffects(leds, FILLIN, FACE_MIN, FACE_MAX, 4, 15, 0xffffff, false, FORWARD);
#endif
}

void resetEffects() {
  int i;
  for (i = 1; i < 5; ++i) {
    effect[i].setEffect(FILLIN);
    effect[i].setColor((CRGB)0x008EFF);
  }

  effect[0].setEffect(FILLIN);
  effect[0].setColor((CRGB)0xFFFFFF);
}

void setup() { 
  int i;

  // Serial init
  while (!Serial) ;
  Serial.begin(SERIAL_BAUDRATE);
//  Serial.setTimeout(200);
  
  // Laser init
  lasers.laserPin = LASER_DMX_PIN;
  lasers.begin(94); //Pick a start pattern

  // Control Pins Init
  pinMode(LASER_ENABLE_PIN, OUTPUT);
  digitalWrite(LASER_ENABLE_PIN, HIGH);

  // Neopixel init
  FastLED.addLeds<NEOPIXEL,DATA_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(MAX_BRIGHTNESS);

  // Init bottom LEDs
  FastLED.addLeds<NEOPIXEL, LED1>(&lowerLeds[0][0], 3);
  FastLED.addLeds<NEOPIXEL, LED2>(&lowerLeds[1][0], 3);
  FastLED.addLeds<NEOPIXEL, LED3>(&lowerLeds[2][0], 3);
  FastLED.addLeds<NEOPIXEL, LED4>(&lowerLeds[3][0], 3);

  InitEffects();

#ifdef INIT_NONE
  for (i = 0; i < 4; ++i) {
    lowerLeds[i][0] = (CRGB)0;
    lowerLeds[i][1] = (CRGB)0;
    lowerLeds[i][2] = (CRGB)0;   
  }
#else
  for (i = 0; i < 4; ++i) {
    lowerLeds[i][0] = (CRGB)0x008EFF;
    lowerLeds[i][1] = (CRGB)0x008EFF;
    lowerLeds[i][2] = (CRGB)0x008EFF;   
  }
#endif

  Serial.println("pewpew ready.");  
}

uint32_t getHex() {
  int i;
  uint32_t val = 0;
  for (i = loc; i < MAX_CMD_SIZE - loc || i < 6; ++i) {
    uint8_t c = cmd[i];
    if ( (c >= '0') && ( c <= 'f')) {
      val = 16 * val + c - '0';
    } else {
      break;
    }
  }
  loc += 6;
  return val;
}

uint32_t getInt() {
  int i;
  uint32_t val = 0;
  for (i = loc; i < MAX_CMD_SIZE - loc; ++i) {
    uint8_t c = cmd[i];
    if ( (c >= '0') && ( c<= '9')) {
      val = 10 * val + c - '0';
    } else {
      break;
    }
  }
  loc += i-loc;
  return val;
}

char getChar() {
  return cmd[loc];
}

void updateEffects() {
   uint8_t code, selected = 0;
   uint32_t val;
   int i;
   NeoPixelEffects* effectList[10] = { 0 };
   uint8_t effectSize = 0;
   
//   Serial.println("Updating Neopixel");
   
   // Grab offset (1-12)
   code = getInt();

    if (code < 0 || code > 14) {
//      Serial.println("WARN: Invalid ring. Range is 1-4");
      return;
    } else if (code <= 4) {
      effectList[0] = &effect[code];
      effectSize = 1;
    } else if (code > 4) { // update multiple areas
      // Begin shitty graph
      switch (code) {
        case ALL: { // everything
          effectList[0] = &effect[RING1];
          effectList[1] = &effect[RING2];
          effectList[2] = &effect[RING3];
          effectList[3] = &effect[RING4];
          effectList[4] = &effect[FACE];
          effectSize = 5;
          break;
        }
        case HALFR: {
          effectList[0] = &effect[RING1];
          effectList[1] = &effect[RING2];
          effectSize = 2;
          break;
        }
        case HALFL: {
          effectList[0] = &effect[RING3];
          effectList[1] = &effect[RING4];
          effectSize = 2;
          break;
        }
        case DIAGR: {
          effectList[0] = &effect[RING1];
          effectList[1] = &effect[RING3];
          effectSize = 2;
          break;
        }
        case DIAGL: {
          effectList[0] = &effect[RING2];
          effectList[1] = &effect[RING4];
          effectSize = 2;
          break;
        }
        case BACK: {
          effectList[0] = &effect[RING1];
          effectList[1] = &effect[RING4];
          effectSize = 2;
          break;
        }
        case FRONT: {
          effectList[0] = &effect[RING2];
          effectList[1] = &effect[RING3];
          effectSize = 2;
          break;
        }
        case NOHEAD: {
          effectList[0] = &effect[RING1];
          effectList[1] = &effect[RING2];
          effectList[2] = &effect[RING3];
          effectList[3] = &effect[RING4];
          effectSize = 4;
          break;
        }
      }
    }
  
    // Decode effect.
    code = cmd[loc++];

    switch (code) {
      // Effect Select
      case 'e': 
        val = getInt();
        if (val < 0 || val > NUM_EFFECT) {
//          Serial.print("Invalid Effect: ");
//          Serial.println(val, DEC);
          return;
        }
//        Serial.println("Setting Effect");
//        Serial.println(val);
        for (i = 0; i < effectSize; ++i) {
          effectList[i]->setEffect((Effect)val); 
        }
        break;

      // Color
      case 'c':
        val = getHex();
//        Serial.println("Setting Color");
//        Serial.println(val);
        for (i = 0; i < effectSize; ++i) {
          effectList[i]->setColor((CRGB)val);
        }
        break;

      // Range (r45r35)
      // FIXME ?
//      case 'r': {
//        uint16_t lower, upper;
//        lower = getInt();
//        Serial.println(loc);
//        if (cmd[loc] != 'r') {
//          Serial.println("WARN: On NeoPixel Range, expected 'r', got something else.");
//          return;
//        }
//        loc++;
//        upper = getInt();
//        Serial.println("Setting Range");
//        Serial.println(lower);
//        Serial.println(upper);
//        effect[selected].setRange(lower, upper);
//       }
//        break;

      // Area of effect
      case 'a': 
        val = getInt();
//        Serial.println("Setting AoE");
//        Serial.println(val);
        for (i = 0; i < effectSize; ++i) {
          effectList[i]->setAreaOfEffect(val);
        }
        break;

      // Delay
      case 'd':
        val = getInt();
//        Serial.println("Setting Delay");
//        Serial.println(val);
        for (i = 0; i < effectSize; ++i) {
          effectList[i]->setDelay(val);
        }
        break;

      // Fill in solid 
      case 'f':
        val = getHex();
        for (i = 0; i < effectSize; ++i) {
          effectList[i]->fill_solid((CRGB)val);
        }
        break;

      // Fill in gradient 
      // FIXME
//      case 'g': {
//        uint32_t val1 = getHex();
//        val = getHex();
//        Serial.println(val1);
//        Serial.println(val);
//        for (i = 0; i < effectSize; ++i) {
//          effectList[i]->fill_gradient((CRGB)val1, (CRGB)val);
//        }
//      }
//        break;

      // Set looping
      case 'l':
        val = getInt();
//        Serial.println("Enabling/Disabling Loop");
        if (val != 0) {
//          Serial.println("true"); 
          for (i = 0; i < effectSize; ++i) {
            effectList[i]->setRepeat(1);  
          }
        } else {
//          Serial.println("false");
          for (i = 0; i < effectSize; ++i) {
            effectList[i]->setRepeat(0);  
          }
        }
        break;

      // Set invert
      case 'i':
        val = getInt();
//        Serial.println("Setting Direction");
        if (val != 0) {
//          Serial.println("true");
          for (i = 0; i < effectSize; ++i) {
            effectList[i]->setDirection(1);
          }
        } else {
//          Serial.println("false");
          for (i = 0; i < effectSize; ++i) {
            effectList[i]->setDirection(0);
          }
        }
        break;
    }
}

void updateCtrl() {
  switch (cmd[loc++]) {
    // shutdown EVERYTHING
    case '0': break;

    // turn on EVERYTHING
    case '1': break;

    // clear neopixels
    case '2':{
      effect[RING1].clear();
      effect[RING2].clear();
      effect[RING3].clear();
      effect[RING4].clear();
      effect[FACE].clear();  
      effect[RING1].stop();
      effect[RING2].stop();
      effect[RING3].stop();
      effect[RING4].stop();
      effect[FACE].stop();  
    } break;

    // reinit neopixels
    case '3': resetEffects(); break;

    // turn off lasuh
    case '4': digitalWrite(LASER_ENABLE_PIN, LOW); break;

    // turn on lasuh
    case '5': digitalWrite(LASER_ENABLE_PIN, HIGH); break;
    
    default: 
      Serial.print("Ctrl cmd not recognized: ");
      Serial.println(cmd[loc-1]);
  }
}

void updateLeds() {
  uint8_t code = cmd[loc++] - '0';
  uint32_t hex;

  if (code < 0 || code > 4) {
    Serial.println("Invalid LED range.");
    return;
  }

  hex = getHex();

  if (code == 0) {
    int i;
    for (i = 0; i < 4; ++i) {
      lowerLeds[i][0] = hex;
      lowerLeds[i][1] = hex;
      lowerLeds[i][2] = hex;
    }
  } else {
    lowerLeds[code-1][0] = hex;
    lowerLeds[code-1][1] = hex;
    lowerLeds[code-1][2] = hex; 
  }
}

void readCmd() {
  uint8_t code;
  loc = 0;
  int i;
  
//  Serial.println(cmd);
  code = cmd[loc++];

  switch (code) {
    // Update control.
    case 'c': updateCtrl(); break;
      
    // Update leds.
    case 'p': updateLeds(); break;
     
    // Update lasers.
    case 'l': lasers.laserComs(cmd, &loc); break;
  
    // Update neopixel.
    case 'n': updateEffects(); break;
  
    default: 
      Serial.print("Warn: Unrecognized initial character: ");
      Serial.println(code);
  }
}

void loop() {
  int r = 0;
  currTime = micros();

  // XXX
  // Process serial command. Updated this routine due to the serial read issues. 
  // You can try it for yourself. If you remove the showLED time throttle, 
  // Allowing it to update as fast as possible, the serial almost always skips
  // characters on a read. As this is interrupt driven serial, that seems to 
  // make no sense. But it's happening. Even at 50000 throttle, the read skips
  // every once in a while. Any insight on this issue is much appreciated.
  // The serial read works flawlessly when fastLED.show() is not called at all. 
  if ((r = Serial.available())) {
    int i;
    for (i = 0; i < r; ++i) {
      int c = Serial.read();
      if (c != ';' && c != '\0' && readchar < MAX_CMD_SIZE) {
        cmd[readchar++] = c;  
      } else {
        readyforCmd = true;
        break;
      }      
    }
    
    if (readyforCmd) {
      // Input
      Serial.println(cmd);
      readCmd();
      for (i = 0; i < MAX_CMD_SIZE; ++i) {
        cmd[i] = '\0';
      }
      readyforCmd = false;   
      readchar = 0;
      Serial.println("ACK");       
    }
  }

  // Update routine for effects. 
  effect[RING1].update();
  effect[RING2].update();
  effect[RING3].update();
  effect[RING4].update();
  effect[FACE].update();  

//  Serial.println(FastLED.getFPS());

  if ((currTime - prevTime) > UPDATE_PERIOD) {
    prevTime = currTime;
    FastLED.show();   
  }
}


