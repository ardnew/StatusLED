# StatusLED
Single-LED RGB driver for Arduino, intended for use with onboard Neopixel/DotStar/RGB LEDs.

Provides 3 operating modes:
1. `Fixed` - set color with given brightness
2. `Pulse` - continuously cycle brightness of a given color (sometimes called "breathe")
3. `Fabulous` - cycle colors using the ubiquitous `Wheel` algorithm

Uses PWM on three separate R,G,B GPIO pins by default, but very easy to adapt to use with other driver libraries such as FastLED, Adafruit_DotStar, etc.

## Example
Demonstrates dynamically changing the LED mode, using the onboard RGB LED on an Arduino Nano 33 BLE Sense:
```c++
#include "src/StatusLED.h"

StatusLED *led;

void setup()
{
  led = new StatusLED(
      LEDR, LEDG, LEDB,
      StatusLEDMode::Pulse,
      true, // show/enable
      COLOR_GREEN,
      0xFF, // max brightness
      5,    // delay (ms)
      1     // step (wheel/pulse delta)
  );
}

void loop()
{
  static uint32_t start = millis();
  static int mode = 0;

  switch (mode) {
    case 0:
      if (millis() - start > 5000) {
        led->fabulous(0xF0, 5/*delay*/, 1/*step*/);
        ++mode;
      }
      break;

    case 1:
      if (millis() - start > 10000) {
        led->fixed(COLOR_YELLOW, 0xFF);
        ++mode;
      }
      break;

    case 2:
      if (millis() - start > 12000) {
        led->pulse(COLOR_BLUE, 0xFF, 1/*delay*/, 2/*step*/);
        ++mode;
      }
      break;

    case 3:
      if (millis() - start > 17000) {
        led->show(false);
        ++mode;
      }
      break;

    case 4:
      if (millis() - start > 20000) {
        led->show(true);
        ++mode;
      }
      break;
  }

    led->update();
}
```
