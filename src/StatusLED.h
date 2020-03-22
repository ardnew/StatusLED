/*******************************************************************************
 *
 *  name: StatusLED.h
 *  date: Mar 22, 2020
 *  auth: andrew
 *  desc:
 *
 ******************************************************************************/

#ifndef __STATUS_LED_H__
#define __STATUS_LED_H__

// ----------------------------------------------------------------- includes --

#include "global.h"

// ------------------------------------------------------------------ defines --

#define COLOR_BLACK       SRGB{ red:   0, green:   0, blue:   0 }
#define COLOR_NAVY        SRGB{ red:   0, green:   0, blue: 123 }
#define COLOR_DARKGREEN   SRGB{ red:   0, green: 125, blue:   0 }
#define COLOR_DARKCYAN    SRGB{ red:   0, green: 125, blue: 123 }
#define COLOR_MAROON      SRGB{ red: 123, green:   0, blue:   0 }
#define COLOR_PURPLE      SRGB{ red: 123, green:   0, blue: 123 }
#define COLOR_OLIVE       SRGB{ red: 123, green: 125, blue:   0 }
#define COLOR_LIGHTGREY   SRGB{ red: 198, green: 195, blue: 198 }
#define COLOR_DARKGREY    SRGB{ red: 123, green: 125, blue: 123 }
#define COLOR_BLUE        SRGB{ red:   0, green:   0, blue: 255 }
#define COLOR_GREEN       SRGB{ red:   0, green: 255, blue:   0 }
#define COLOR_CYAN        SRGB{ red:   0, green: 255, blue: 255 }
#define COLOR_RED         SRGB{ red: 255, green:   0, blue:   0 }
#define COLOR_MAGENTA     SRGB{ red: 255, green:   0, blue: 255 }
#define COLOR_YELLOW      SRGB{ red: 255, green: 255, blue:   0 }
#define COLOR_WHITE       SRGB{ red: 255, green: 255, blue: 255 }
#define COLOR_ORANGE      SRGB{ red: 255, green: 165, blue:   0 }
#define COLOR_GREENYELLOW SRGB{ red: 173, green: 255, blue:  41 }
#define COLOR_PINK        SRGB{ red: 255, green: 130, blue: 198 }

#define WHEEL_DELTA 0x01
#define PULSE_DELTA 0x01

// ------------------------------------------------------------------- macros --

/* nothing */

// ----------------------------------------------------------- exported types --

class SRGB {
private:
  inline int16_t clip(int16_t const c) const
    { return (c < 0) ? 0 : ( (c > 255) ? 255 : c ); }

public:
  int16_t red, green, blue;

  inline SRGB Clipped(void) const
    { return SRGB{ red:clip(red), green:clip(green), blue:clip(blue) }; }
};

class StatusRGB {
public:
  // 16-bit to support the pulse fading logic
  int16_t red;
  int16_t green;
  int16_t blue;

  StatusRGB(void):
    red(0), green(0), blue(0)
  {}
  StatusRGB(int16_t r, int16_t g, int16_t b):
    red(r), green(g), blue(b)
  {}
  StatusRGB(SRGB const &rgb):
    red(rgb.red), green(rgb.green), blue(rgb.blue)
  {}

  inline SRGB RGB(void) const
    { return SRGB{ red: red, green: green, blue: blue }; }
  inline SRGB RGBClipped(void) const
    { return RGB().Clipped(); }

  inline bool operator==(StatusRGB const &rgb) const
    { return (red == rgb.red) && (green == rgb.green) && (blue == rgb.blue); }
  inline bool operator!=(StatusRGB const &rgb) const
    { return (red != rgb.red) || (green != rgb.green) || (blue != rgb.blue); }
};

enum class StatusLEDMode {
  NONE = -1,
  Fixed,    // = 0
  Pulse,    // = 1
  Fabulous, // = 2
  COUNT     // = 3
};

class StatusLED
{
private:
  uint16_t       _redPin;
  uint16_t       _greenPin;
  uint16_t       _bluePin;
  StatusLEDMode  _mode;   // the color change mode
  bool           _show;   // whether or not neopixel is showing
  StatusRGB      _color;  // current color of pixel
  uint8_t        _bright; // total brightness (0x00 - 0xFF)
  uint32_t       _delay;  // time to wait between color updates
  int8_t         _step;   // pulse/wheel color delta
  uint32_t       _last;   // time of last color update
  StatusRGB      _pulse;  // current pulse color
  StatusRGB      _wheel;  // current wheel color
  bool           _change; // flag indicating we need to call show()
  bool           _chmod;  // flag indicating we need to reset pulse/wheel

  void setShow(bool const show, bool const force = false);
  int8_t nextPulse(int8_t const dir, int8_t const step);
  uint8_t nextWheel(uint8_t const pos, int8_t const step);
  SRGB scale(SRGB const &rgb, uint8_t const scale) const;
  void write(SRGB const &rgb) const;

public:
  StatusLED(
      uint16_t const redPin,
      uint16_t const greenPin,
      uint16_t const bluePin,
      StatusLEDMode const mode,
      bool const show,
      SRGB const rgb,
      uint8_t const bright,
      uint32_t const delay,
      int8_t const step
  );

  void update(void);

  void show(bool const show);
  void fixed(SRGB const &rgb, uint8_t const bright);
  void pulse(SRGB const &rgb, uint8_t const bright, uint32_t const delay, int8_t const step = PULSE_DELTA);
  void fabulous(uint8_t const bright, uint32_t const delay, int8_t const step = WHEEL_DELTA);

};

#endif // __STATUS_LED_H__
