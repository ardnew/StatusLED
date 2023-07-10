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

#include <functional>

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
#define COLOR_LIGHTGREEN  SRGB{ red: 173, green: 255, blue: 198 }
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

  static constexpr uint8_t _gamma8[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
    2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
    5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
   10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
   17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
   25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
   37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
   51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
   69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
   90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
  115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
  144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
  177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };
public:
  int16_t red, green, blue;

  inline SRGB Clipped(void) const
    { return SRGB{ red:clip(red), green:clip(green), blue:clip(blue) }; }
  inline SRGB Gamma8(void) const {
    return SRGB{
      red:   _gamma8[red],
      green: _gamma8[green],
      blue:  _gamma8[blue],
    };
  }
  inline String Desc(void) const {
    char s[16];
    snprintf(s, sizeof(s), "{ %02X, %02X, %02X }", red, green, blue);
    return String(s);
  }
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

  inline String Desc(void) const
    { return RGB().Desc(); }

  inline bool operator==(StatusRGB const &rgb) const
    { return (red == rgb.red) && (green == rgb.green) && (blue == rgb.blue); }
  inline bool operator!=(StatusRGB const &rgb) const
    { return (red != rgb.red) || (green != rgb.green) || (blue != rgb.blue); }
};

enum class StatusLEDMode {
  NONE = -1,
  Fixed,    // = 0
  Blink,    // = 1
  Pulse,    // = 2
  Fabulous, // = 3
  COUNT     // = 4
};

typedef std::function<void(SRGB const &)> WriteColor;

class StatusLED
{
protected:
  uint16_t       _redPin;
  uint16_t       _greenPin;
  uint16_t       _bluePin;
  bool           _offLevel;  // Logic level to turn off LED
  StatusLEDMode  _mode;      // the color change mode
  uint32_t       _frequency; // time to wait between color updates
  bool           _show;      // whether or not LED is illuminated
  StatusRGB      _color;     // current color of LED
  uint8_t        _bright;    // total brightness (0x00 - 0xFF)
  uint32_t       _period;    // time between each blink
  uint32_t       _onDuty;    // illumination duration each blink period
  int8_t         _step;      // pulse/wheel color delta
  StatusRGB      _blink;     // current blink color
  StatusRGB      _pulse;     // current pulse color
  StatusRGB      _wheel;     // current wheel color
  bool           _change;    // flag indicating we need to call show()
  bool           _chmod;     // flag indicating we need to reset mode
  WriteColor     _write;

  void setShow(bool const show, bool const force = false);
  bool nextBlink(bool const isOn, uint32_t const curr, uint32_t * const sync);
  int8_t nextPulse(int8_t const dir, int8_t const step);
  uint8_t nextWheel(uint8_t const pos, int8_t const step);
  SRGB scale(SRGB const &rgb, uint8_t const scale) const;
  void analogWrite(SRGB const &rgb) const;

public:
  StatusLED(
      uint16_t const redPin,
      uint16_t const greenPin,
      uint16_t const bluePin,
      bool const offLevel,
      StatusLEDMode const mode,
      uint32_t const frequency,
      bool const show,
      SRGB const rgb,
      uint8_t const bright = 0x80
  );

  void setWrite(WriteColor write);
  void update(uint32_t curr = millis());

  void show(bool const show);
  void fixed(SRGB const &rgb, uint8_t const bright);
  void blink(SRGB const &rgb, uint8_t const bright, uint32_t const period, uint32_t const onDuty);
  void pulse(SRGB const &rgb, uint8_t const bright, uint32_t const frequency, int8_t const step = PULSE_DELTA);
  void fabulous(uint8_t const bright, uint32_t const frequency, int8_t const step = WHEEL_DELTA);

};

#endif // __STATUS_LED_H__
