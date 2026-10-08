/***************************************************
 * ESP32-Eyes, Copyright (c) 2023 Alastair Aitchison, Playful Technology
 * Based on previous work by Luis Llamas (www.luisllamas.es)
 *
 ****************************************************/

// INCLUDES
// Built-in Arduino I2C library
#include <Wire.h>
// Defines all face functionality
#include "Face.h"

// CONSTANTS
// Pin assignments below target the ESP32-C3 Super Mini. Its usable GPIOs are
// 0-10 and 20-21; GPIO8/9 default to I2C (SDA/SCL, see Face.cpp). Only
// GPIO0-4 are wired to the chip's ADC1 unit: GPIO5 is the C3's only ADC2
// channel, and Arduino-ESP32 refuses analogRead() on it (ADC2 shares
// circuitry with Wi-Fi and is disabled by default on this chip), so all 5
// analog inputs below are kept within GPIO0-4. GPIO2 is a strapping pin,
// but its state is only latched at boot when GPIO9 is pulled low (download
// mode); since GPIO9 is used here as the I2C SCL line with its default
// pull-up, it stays high on reset, so GPIO2's strap value is ignored and
// it's safe to use as a regular analog input.
const byte joystickPins[] = {0, 1}; // {Y axis, X axis}
const byte blinkPin = 10;
const byte moodPins[] = {2, 3, 4};
const eEmotions moods[] = {eEmotions::Angry, eEmotions::Sad, eEmotions::Surprised}; 

// GLOBALS
Face *face;

void setup(void) {
  // Create a serial connection
  Serial.begin(115200);
  Serial.println(__FILE__ __DATE__);

  // Configure input pins
  pinMode(blinkPin, INPUT_PULLUP);

  // Create a new face
  face = new Face(/* screenWidth = */ 128, /* screenHeight = */ 64, /* eyeSize = */ 40);
  // Assign the current expression
  face->Expression.GoTo_Normal();

  // Assign a weight to each emotion
  //face->Behavior.SetEmotion(eEmotions::Normal, 1.0);
  //face->Behavior.SetEmotion(eEmotions::Angry, 1.0);
  //face->Behavior.SetEmotion(eEmotions::Sad, 1.0);
  // Automatically switch between behaviours (selecting new behaviour randomly based on the weight assigned to each emotion)
  face->RandomBehavior = true;

  // Automatically blink
  face->RandomBlink = true;
  // Set blink rate
  face->Blink.Timer.SetIntervalMillis(4000);

  // Automatically choose a new random direction to look
  face->RandomLook = false;
}

float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
  // Check if x is outside the input range
  if (x <= in_min) return out_min;
  if (x >= in_max) return out_max;

  // Calculate the proportion of x relative to the input range
  float proportion = (x - in_min) / (in_max - in_min);

  // Map the proportion to the output range and return the result
  return (proportion * (out_max - out_min)) + out_min;
}

void loop(){
  static int lastMoveTime;

  // To avoid making eyes too twitchy (and to allow time for previous move animation to end),
  // only recalculate new position every 500ms
  if(millis() - lastMoveTime > 500) {
    int yRaw = analogRead(joystickPins[0]);
    int xRaw = analogRead(joystickPins[1]);
    float y = mapFloat(yRaw, 0, 4095, 1.0, -1.0);
    float x = mapFloat(xRaw, 0, 4095, -1.0, 1.0);
    face->Look.LookAt(x, y);
    lastMoveTime = millis();
  }
  // Blink when joystick pressed in
  if(!digitalRead(blinkPin)){
    face->DoBlink();
  }

  // Use this code to set a particular emotion from a button
  for(int i=0; i<3; i++){
    int32_t potValue = analogRead(moodPins[i]);
    float moodLevel = map(potValue, 0, 4095, 0.0, 2.0);
    face->Behavior.SetEmotion(moods[i], moodLevel);
  }

  // Update the face!
  face->Update();
}