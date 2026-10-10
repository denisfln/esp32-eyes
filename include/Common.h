#ifndef COMMON_h
#define COMMON_h

#include <U8g2lib.h>

// Two physical SSD1306 displays share the same hardware I2C bus
// (SDA=GPIO8, SCL=GPIO9), distinguished by I2C address: the left eye's
// display is wired/configured to 0x3C, the right eye's to 0x3D.
extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2Left;
extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2Right;

#endif