# esp32-eyes
Emotive animated eyes on an OLED display, as inspired by Anki Cozmo etc.

Inspired by the expressive eye animations of the Anki "Cozmo" robot, this Arduino/ESP8266/ESP32 library displays a set of animated eyes on a small 128x64 OLED screen.

<img src="https://github.com/playfultechnology/esp32-eyes/blob/main/doc/anki-cozmo-faces-3-1024x576.jpg" />


Unlike some libraries which display a set of pre-rendered bitmap images for each frame of animation, this library draws each frame dynamically from a programmatic set of parameters.

Heavily based on <a href="https://github.com/luisllamasbinaburo/ESP32_Faces/">this library</a>, although with significant adjustments:

 - OLED (using <a href="https://github.com/olikraus/u8g2">u8g2</a>) rather than TFT (based on <a href="https://github.com/Bodmer/TFT_eSPI">eSPI_TFT</a>)
 - Horizontal rather than vertical alignment
 - Mirrored left/right eye animations
 - Automatic or manual blink and look direction

## Hardware Used
This fork targets the **ESP32-C3 Super Mini** board, using:
 - ESP32-C3 Super Mini - two generic SSD1306 128x64 I2C OLEDs are wired up externally (the board has no built-in display), one per eye
 - 3x linear potentiometers - https://www.aliexpress.com/item/1005005859787527.html
 - Joystick module - https://www.aliexpress.com/item/32901984938.html

<img src="https://github.com/playfultechnology/esp32-eyes/blob/main/doc/Schematic_ESP32-OLED-Eyes.png" />

### ESP32-C3 Super Mini pinout
The Super Mini only exposes GPIO0-10 and GPIO20/21, and only GPIO0-4 are wired
to the chip's ADC1 unit, so wiring differs from the original ESP32 schematic
above. GPIO5 is the C3's only ADC2 channel; Arduino-ESP32 disables
`analogRead()` on it on this chip (ADC2 shares circuitry with Wi-Fi and is
unreliable), so it is **not** used for any analog input:

| Signal                        | GPIO |
|--------------------------------|------|
| OLED SDA                      | 8    |
| OLED SCL                      | 9    |
| Joystick Y axis                | 0    |
| Joystick X axis                | 1    |
| Joystick button (blink)        | 10   |
| Mood potentiometer - Angry      | 2    |
| Mood potentiometer - Sad        | 3    |
| Mood potentiometer - Surprised  | 4    |

The ESP32-C3 only has a single hardware I2C peripheral, so both OLED
displays share the same SDA/SCL pins above and are wired in parallel (same
VCC/GND too). They are distinguished by I2C address instead: the left eye's
display must be configured (via its address-select jumper/resistor, if
present) to `0x3C`, and the right eye's display to `0x3D`.

GPIO8 and GPIO9 are boot strapping pins used here for I2C (SDA/SCL default
to a pull-up, matching their required state at reset). GPIO2 is also a
strapping pin, but its strap value is only latched when GPIO9 is pulled low
(download boot mode); since GPIO9 stays high here, GPIO2 is safe to use as a
regular analog input.

## Building with PlatformIO
This project is a standard [PlatformIO](https://platformio.org/) project targeting
`env:esp32-c3-supermini` (see `platformio.ini`). Since the Super Mini has no
USB-to-serial bridge chip, `platformio.ini` enables the native USB CDC stack
so `Serial` output works over the USB port. Build and upload with:
```
pio run -t upload
pio device monitor
```

## Software Component Structure

- *Face* is the core object, consisting of left and right eye components, behaviour, expression, a look assistant, and a blink assistant. The outputs of these components are chained together to create the display on each frame:

- The Face *Behaviour* contains an array of weighted _Emotions_. There are currently 18 emotions, as defined in the enum in FaceEmotions.hpp. 
Each emotion can have an individual weight assigned to it, as follows: 
```
face->Behavior.SetEmotion(eEmotions::Glee, 1.0);
face->Behavior.SetEmotion(eEmotions::Anger, 0.5);
```
If ```face->RandomBehavior = true;``` (which is the default), then Face will use a roulette-wheel selection to randomly assign a new emotion based on their relative weights.
The frequency with which the current emotion is re-evaluated is determined by the value passed to ```face->Behaviour.Timer.SetIntervalMillis();``` , although note that this does not always result in a change, since the same emotion may be selected as is currently active.
If no weights have been assigned to any emotion, the face will default to "normal" emotion.

Note that emotions are _not_ blended together - the face only ever expresses one emotion at a time. The "weight" assigned to each emotion does not specify how they are composited together, but rather it states the relative probability with which any given emotion will be selected.

- *Expression* handles transitions of the eyes to a selected emotion. Each emotion has a preset configuration that defines the eye height, slope, radius, and offset for each emotion (defined in ```EyePresets.h```), and these are assigned by the corresponding ```GoTo_Emotion()``` function.

- The Blink assistant handles blinking. If ```face->RandomBlink = true;``` (the default), the eyes will automatically blink, with a frequency determined by the value passed to ```face->Blink.Timer.SetIntervalMillis();```
The eyes may be blinked manually by calling ```face->DoBlink();```

- The Look assistant handles the global position of the eyes around the screen. If ```face->RandomLook = true;``` (the default), the eyes will randomly move around the screen, with a frequency determined by the value passed to ```face->Look.Timer.SetIntervalMillis();```
The eyes may be set to look at any position by calling ```face->Look.LookAt(x, y);``` (where x and y range from -1.0 - 1.0) 


- Each eye is responsible for managing their own drawing operations to the display (in EyeDrawer.cpp)


