/***************************************************
Copyright (c) 2020 Luis Llamas
(www.luisllamas.es)

This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by
the Free Software Foundation, either version 3 of the License, or (at your option) any later version. 

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses 
****************************************************/


#include <Wire.h>
// Needed for i2c_set_timeout() - see explanation below.
#include <driver/i2c.h>
#include "Face.h"
#include "Common.h"

// SCL=GPIO9 / SDA=GPIO8 match the ESP32-C3 Super Mini's (and Arduino-ESP32's)
// default I2C pin assignment. Both displays share this same hardware I2C
// bus (the ESP32-C3 only has a single I2C peripheral) and are distinguished
// by their I2C address instead.
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2Left(U8G2_R0,  U8X8_PIN_NONE);
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2Right(U8G2_R0, U8X8_PIN_NONE);

Face::Face(uint16_t screenWidth, uint16_t screenHeight, uint16_t eyeSize) 
	: LeftEye(*this, u8g2Left), RightEye(*this, u8g2Right), Blink(*this), Look(*this), Behavior(*this), Expression(*this) {

  // Arduino-ESP32's I2C driver (as of core 2.0.x) always configures the
  // hardware clock-stretch timeout to its chip-specific maximum when
  // Wire.begin() runs. On the ESP32-C3 that timeout register is only 5
  // bits wide, and the valid argument to i2c_set_timeout() on this chip is
  // that raw 0-31 register value (not a literal clock-cycle count, despite
  // what the function's doc comment says - passing a literal cycle count
  // like 20000 is rejected by the driver's internal range check and logs
  // "i2c timing value error"). The register value is interpreted as an
  // exponent (2^n core clock cycles), so the library's default of 31 (the
  // max for 5 bits) works out to roughly 2^31 cycles - about 27 *seconds*
  // - for a single clock-stretch event. SSD1306 OLEDs routinely stretch
  // the clock briefly (e.g. while enabling their internal charge pump),
  // which is normal, but it means such a moment can block for tens of
  // seconds instead of microseconds. Explicitly starting the bus here
  // (instead of letting u8g2 call Wire.begin() implicitly) lets us
  // immediately override that timeout with a sane value before any display
  // communication occurs.
  Wire.begin(8, 9);
  Wire.setClock(100000);
  Wire.setTimeOut(50);   // Timeout em milissegundos
  //i2c_set_timeout(I2C_NUM_0, 20); // 2^20 cycles =~ 13ms at 80MHz APB clock - plenty for normal clock stretching

  // Unlike almost every other Arduino library (and the I2C address scanner script etc.)
  // u8g2 uses 8-bit I2C address, so we shift the 7-bit address left by one
  Serial.println("  Initializing left display (0x3C)...");
  u8g2Left.setI2CAddress(0x3C<<1);
  Serial.println("  Starting begin left display (0x3C)...");
  u8g2Left.begin();
  Serial.println("  Finishing begin left display (0x3C)...");
  u8g2Left.clearBuffer();
  Serial.println("  Left display initialized.");

  Serial.println("  Initializing right display (0x3D)...");
  u8g2Right.setI2CAddress(0x3D<<1);
  Serial.println("  Starting begin right display (0x3D)...");
  u8g2Right.begin();
  Serial.println("  Finishing begin right display (0x3D)...");
  u8g2Right.clearBuffer();
  Serial.println("  Right display initialized.");

	Width = screenWidth;
	Height = screenHeight;
	EyeSize = eyeSize;

	CenterX = Width / 2;
	CenterY = Height / 2;

	LeftEye.IsMirrored = true;

  Behavior.Clear();
	Behavior.Timer.Start();
}

void Face::LookFront() {
	Look.LookAt(0.0, 0.0);
}

void Face::LookRight() {
	Look.LookAt(-1.0, 0.0);
}

void Face::LookLeft() {
	Look.LookAt(1.0, 0.0);
}

void Face::LookTop() {
	Look.LookAt(0.0, 1.0);
}

void Face::LookBottom() {
	Look.LookAt(0.0, -1.0);
}

void Face::Wait(unsigned long milliseconds) {
	unsigned long start;
	start = millis();
	while (millis() - start < milliseconds) {
		Draw();
	}
}

void Face::DoBlink() {
	Blink.Blink();
}

void Face::Update() {
	if(RandomBehavior) Behavior.Update();
	if(RandomLook) Look.Update();
	if(RandomBlink)	Blink.Update();
	Draw();
}

void Face::Draw() {
  // Each eye now has its own dedicated display, so it is simply centered
  // on that display rather than being offset within a single shared screen.
	LeftEye.CenterX = CenterX;
	LeftEye.CenterY = CenterY;
	RightEye.CenterX = CenterX;
	RightEye.CenterY = CenterY;

  // Clear, draw, and transfer the buffer for the left eye's display
  u8g2Left.clearBuffer();
	LeftEye.Draw();
  u8g2Left.sendBuffer();

  // Clear, draw, and transfer the buffer for the right eye's display
  u8g2Right.clearBuffer();
	RightEye.Draw();
  u8g2Right.sendBuffer();
}
