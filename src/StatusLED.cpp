/*******************************************************************************
 *
 *  name: StatusLED.cpp
 *  date: Mar 22, 2020
 *  auth: andrew
 *  desc:
 *
 ******************************************************************************/

// ----------------------------------------------------------------- includes --

#include <Arduino.h>

#include "StatusLED.h"

// ---------------------------------------------------------- private defines --

#ifndef PRINTF_DEBUG_MAX_LEN
  // comment to disable debug logging by default
  //#define PRINTF_DEBUG_MAX_LEN 1024
#endif

// must be defined and non-zero
#if PRINTF_DEBUG_MAX_LEN
  #define PRINTF_DEBUG
  #define UART_DEBUG Serial
  #define UART_DEBUG_WAIT(timeout, baud) \
      while (!(UART_DEBUG) && (millis() < (timeout))) \
        { continue; } (UART_DEBUG).begin(baud);
#else
  #define UART_DEBUG_WAIT(timeout, baud) \
      /* debug code omitted */
#endif

#define PULSE_DIMMEST   0x01
#define PULSE_BRIGHTEST 0xFF

// equality comparison tolerance between two floats
#define FLOAT32_ABSTOL 5.0e-6  // 32-bit
#define FLOAT64_ABSTOL 5.0e-14 // 64-bit

// ----------------------------------------------------------- private macros --

#define infof(fmt, ...) dbgf(dlInfo, fmt, __VA_ARGS__)
#define warnf(fmt, ...) dbgf(dlWarn, fmt, __VA_ARGS__)
#define errf(fmt, ...)  dbgf(dlError, fmt, __VA_ARGS__)

// ------------------------------------------------------------ private types --

typedef enum
{
  dlNONE = -1,
  dlInfo,  // = 0
  dlWarn,  // = 1
  dlError, // = 2
  dlCOUNT  // = 3
}
debug_level_t;

// ------------------------------------------------------- exported variables --

/* nothing */

// -------------------------------------------------------- private variables --

/* nothing */

// ---------------------------------------------- private function prototypes --

void dbgf(debug_level_t level, const char *fmt, ...);

// ------------------------------------------------------- exported functions --

StatusLED::StatusLED(
    uint16_t const redPin,
    uint16_t const greenPin,
    uint16_t const bluePin,
    bool const offLevel,
    StatusLEDMode const mode,
    uint32_t const frequency,
    bool const show,
    SRGB const rgb,
    uint8_t const bright
):
  _redPin(redPin),
  _greenPin(greenPin),
  _bluePin(bluePin),
  _offLevel(offLevel),
  _mode(mode),
  _frequency(frequency),
  _show(show),
  _color(StatusRGB(rgb)),
  _bright(bright),
  _period(0UL),
  _onDuty(0UL),
  _step(0UL),
  _blink(StatusRGB(rgb)),
  _pulse(StatusRGB(rgb)),
  _wheel(StatusRGB(rgb)),
  _change(true),
  _chmod(true),
  _write(std::bind(
    &StatusLED::analogWrite, this, std::placeholders::_1
  ))
{
  UART_DEBUG_WAIT(5000, 115200);
  infof("%s", "UART opened, initializing LED pins");
  pinMode(_redPin, OUTPUT);
  pinMode(_greenPin, OUTPUT);
  pinMode(_bluePin, OUTPUT);
  switch (_mode) {
    case StatusLEDMode::Pulse:
      _step = PULSE_DELTA;
      break;
    case StatusLEDMode::Fabulous:
      _step = WHEEL_DELTA;
      break;
  }
  setShow(show, true);
}

void StatusLED::setWrite(WriteColor write)
{
  _write = write;
}

void StatusLED::update(uint32_t curr)
{
  // state variables of each display mode:
  static uint32_t last =   0UL; // Last update (all modes)
  static uint32_t sync =   0UL; // Blink start
  static bool     tgl  = false; // Blink toggle
  static int8_t   dir  =    -1; // Pulse direction
  static uint8_t  pos  =     0; // Fabulous index

  if (!_show)
    { return; }

  uint32_t elap = curr - last;

  if (_frequency != 0 && elap < _frequency)
    { return; }
  last = curr;
  infof("update: %lu (~%lu)", curr, elap);

  StatusRGB color;
  switch (_mode) {
    default:
    case StatusLEDMode::Fixed:
      infof("%s", "fixed");
      color = _color;
      break;

    case StatusLEDMode::Blink:
      if (_chmod)
        { _blink = _color; }
      if (nextBlink(tgl, curr, &sync)) {
        tgl = !tgl;
        infof("%s=%s", "blink",
          tgl ? "on" : "off");

        color = _blink;
        _change = true;
      }
      break;

    case StatusLEDMode::Pulse:
      if (_chmod)
        { _pulse = _color; }
      dir = nextPulse(dir, _step);
      infof("%s=%s[%d]", "pulse",
        dir > 0 ? "up" : "down", _step);
      color = _pulse;
      _change = true;
      break;

    case StatusLEDMode::Fabulous:
      if (_chmod)
        { _wheel = _color; }
      pos = nextWheel(pos, _step);
      infof("%s=%d[%d]", "fabulous",
        pos, _step);
      color = _wheel;
      _change = true;
      break;
  }

  if (!_change) {
    infof("%s\r\n", "no change!");
    return;
  }

  _write(scale(color.RGBClipped(), _bright));

  _change = false;
  _chmod  = false;
}

void StatusLED::show(bool const show)
{
  if (show == _show)
    { return; }

  _change = true;
  _chmod  = true;

  setShow(show);

  update();
}

void StatusLED::fixed(SRGB const &rgb, uint8_t const bright)
{
  StatusRGB color = StatusRGB(rgb);
  if ((StatusLEDMode::Fixed != _mode) || (color != _color) ||
        (bright != _bright)) {
    _change = true;
    _chmod  = true;
  }
  _mode   = StatusLEDMode::Fixed;
  _color  = color;
  _bright = bright;

  setShow(true);

  update();
}

void StatusLED::blink(
    SRGB const &rgb, uint8_t const bright,
    uint32_t const period, uint32_t const onDuty)
{
  StatusRGB color = StatusRGB(rgb);
  if ((StatusLEDMode::Blink != _mode) || (color != _color) ||
        (bright != _bright) || (0UL != _frequency) ||
        (period != _period) || (onDuty != _onDuty)) {
    _change = true;
    _chmod  = true;
  }
  _mode      = StatusLEDMode::Blink;
  _color     = color;
  _bright    = bright;
  _frequency = 0UL; // Would cause glitches; period/onDuty define frequency
  _period    = period;
  _onDuty    = onDuty;

  setShow(true);

  update();
}

void StatusLED::pulse(
    SRGB const &rgb, uint8_t const bright, uint32_t const frequency,
    int8_t const step)
{
  StatusRGB color = StatusRGB(rgb);
  if ((StatusLEDMode::Pulse != _mode) || (color != _color) ||
        (bright != _bright) || (frequency != _frequency) || (step != _step)) {
    _change = true;
    _chmod  = true;
  }
  _mode      = StatusLEDMode::Pulse;
  _color     = color;
  _bright    = bright;
  _frequency = frequency;
  _step      = step;

  setShow(true);

  update();
}

void StatusLED::fabulous(
    uint8_t const bright, uint32_t const frequency, int8_t const step)
{
  if ((StatusLEDMode::Fabulous != _mode) ||
        (bright != _bright) || (frequency != _frequency) || (step != _step)) {
    _change = true;
    _chmod  = true;
  }
  _mode      = StatusLEDMode::Fabulous;
  _bright    = bright;
  _frequency = frequency;
  _step      = step;

  setShow(true);

  update();
}

// -------------------------------------------------------- private functions --

void dbgf(debug_level_t level, const char *fmt, ...)
{
#ifdef PRINTF_DEBUG
  static const char *prefix[dlCOUNT] = {
    "[ ] ", "[*] ", "[!] "
  };
  static char buff[PRINTF_DEBUG_MAX_LEN] = { 0 };

  va_list arg;
  va_start(arg, fmt);
  vsnprintf(buff, PRINTF_DEBUG_MAX_LEN, fmt, arg);
  va_end(arg);

  (UART_DEBUG).print(prefix[level]);
  (UART_DEBUG).println(buff);
#else
  (void)level;
  (void)fmt;
#endif
}

int16_t interp_i16(
    int16_t const x,
    int16_t const x1, int16_t const y1,
    int16_t const x2, int16_t const y2)
{
  float scale = (float)(x - x1);
  float numer = (float)(y2 - x2);
  float denom = (float)(y1 - x1);
  if (fabsf(denom) < FLOAT32_ABSTOL) {
    denom = FLOAT32_ABSTOL;
  }
  return (int16_t)roundf(scale * numer / denom) + x2;
}

void StatusLED::setShow(bool const show, bool const force)
{
  if ((show != _show) || force) {

    _show = show;

    if (!show) {
      digitalWrite(_redPin,   _offLevel);
      digitalWrite(_greenPin, _offLevel);
      digitalWrite(_bluePin,  _offLevel);
    }
  }
}

bool StatusLED::nextBlink(
    bool const isOn, uint32_t const curr, uint32_t * const sync)
{
  uint32_t elap = curr - (sync == nullptr ? _frequency : *sync);
  if (isOn) {
    if (elap >= _onDuty) {
      return true;
    }
  } else {
    if (elap >= _period) {
      *sync = curr;
      return true;
    }
  }
  return false;
}

int8_t StatusLED::nextPulse(int8_t const dir, int8_t const step)
{
#define dim(c) ( (step >= (c)) || ((c) <= PULSE_DIMMEST) )
#define brt(c) ( (step <= (c)) || ((c) >= PULSE_BRIGHTEST) )

#define MIN_PULSE(c) ( dim((c).red) && dim((c).green) && dim((c).blue) )
#define MAX_PULSE(c) ( brt((c).red) && brt((c).green) && brt((c).blue) )

  if (dir < 0) { // decrease lumens
    _pulse.red   -= step;
    _pulse.green -= step;
    _pulse.blue  -= step;
    if (MIN_PULSE(_pulse))
      { return 1; }
  }
  else { // increase lumens
    _pulse.red   += step;
    _pulse.green += step;
    _pulse.blue  += step;
    if (MAX_PULSE(_pulse))
      { return -1; }
  }
  return dir;

#undef dim
#undef brt

#undef MIN_PULSE
#undef MAX_PULSE
}

uint8_t StatusLED::nextWheel(uint8_t const pos, int8_t const step)
{
  uint8_t curr = pos;
  uint8_t next = pos + step;

  curr = 0xFF - curr;
  if (curr < 0x55) {
    _wheel.red   = 0x03 * curr;
    _wheel.green = 0xFF - curr * 0x03;
    _wheel.blue  = 0x00;
  }
  else if (curr < 0xAA) {
    curr -= 0x55;
    _wheel.red   = 0xFF - curr * 0x03;
    _wheel.green = 0x00;
    _wheel.blue  = 0x03 * curr;
  }
  else {
    curr -= 0xAA;
    _wheel.red   = 0x00;
    _wheel.green = 0x03 * curr;
    _wheel.blue  = 0xFF - curr * 0x03;
  }
  return next;
}

SRGB StatusLED::scale(SRGB const &rgb, uint8_t const scale) const
{
  return SRGB{
    red:   interp_i16(rgb.red,   0, 255, 0, scale),
    green: interp_i16(rgb.green, 0, 255, 0, scale),
    blue:  interp_i16(rgb.blue,  0, 255, 0, scale),
  };
}

void StatusLED::analogWrite(SRGB const &rgb) const
{
#define ANALOG_LEVEL(c) (_offLevel ? 0xFF - (c) : (c))
  infof("        R: %d", ANALOG_LEVEL(rgb.red));
  infof("        G: %d", ANALOG_LEVEL(rgb.green));
  infof("        B: %d", ANALOG_LEVEL(rgb.blue));
  // "::" notation to refer to the globally-scoped "analogWrite" function
  ::analogWrite(_redPin,   ANALOG_LEVEL(rgb.red));
  ::analogWrite(_greenPin, ANALOG_LEVEL(rgb.green));
  ::analogWrite(_bluePin,  ANALOG_LEVEL(rgb.blue));
#undef ANALOG_LEVEL
}
