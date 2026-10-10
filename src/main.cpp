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

// GLOBALS
Face *face;

void setup(void) {
  // Create a serial connection
  Serial.begin(115200);
  // Give the native USB CDC port time to enumerate with the host before the
  // first print, otherwise it can be silently dropped.
  delay(2000);
  Serial.println(__FILE__ __DATE__);

  // Create a new face
  face = new Face(/* screenWidth = */ 128, /* screenHeight = */ 64, /* eyeSize = */ 40);
  // Assign the current expression
  face->Expression.GoTo_Normal();

  // Assign a weight to each emotion
  face->Behavior.SetEmotion(eEmotions::Normal, 1.0);
  face->Behavior.SetEmotion(eEmotions::Angry, 1.0);
  face->Behavior.SetEmotion(eEmotions::Sad, 1.0);
  face->Behavior.SetEmotion(eEmotions::Surprised, 1.0);
  face->Behavior.SetEmotion(eEmotions::Glee, 1.0);
  face->Behavior.SetEmotion(eEmotions::Happy, 1.0);
  face->Behavior.SetEmotion(eEmotions::Worried, 1.0);
  face->Behavior.SetEmotion(eEmotions::Focused, 1.0);
  face->Behavior.SetEmotion(eEmotions::Annoyed, 1.0);
  face->Behavior.SetEmotion(eEmotions::Skeptic, 1.0);
  face->Behavior.SetEmotion(eEmotions::Frustrated, 1.0);
  face->Behavior.SetEmotion(eEmotions::Unimpressed, 1.0);
  face->Behavior.SetEmotion(eEmotions::Sleepy, 1.0);
  face->Behavior.SetEmotion(eEmotions::Suspicious, 1.0);
  face->Behavior.SetEmotion(eEmotions::Squint, 1.0);
  face->Behavior.SetEmotion(eEmotions::Furious, 1.0);
  face->Behavior.SetEmotion(eEmotions::Scared, 1.0);
  face->Behavior.SetEmotion(eEmotions::Awe, 1.0);
  
  
  // Automatically switch between behaviours (selecting new behaviour randomly based on the weight assigned to each emotion)
  face->RandomBehavior = true;
  // Change expression every 5 seconds
  face->Behavior.Timer.SetIntervalMillis(3000);

  // Automatically blink
  face->RandomBlink = true;
  // Set blink rate
  //face->Blink.Timer.SetIntervalMillis(4000);

  // Automatically choose a new random direction to look
  face->RandomLook = true;
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

  // Update the face!
  face->Update();
}