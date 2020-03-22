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

#define PULSE_DIMMEST   0x01
#define PULSE_BRIGHTEST 0xFF

// equality comparison tolerance between two floats
#define FLOAT32_ABSTOL 5.0e-6  // 32-bit
#define FLOAT64_ABSTOL 5.0e-14 // 64-bit

// ----------------------------------------------------------- private macros --

/* nothing */

// ------------------------------------------------------------ private types --

/* nothing */

// ------------------------------------------------------- exported variables --

/* nothing */

// -------------------------------------------------------- private variables --

/* nothing */

// ---------------------------------------------- private function prototypes --

/* nothing */

// ------------------------------------------------------- exported functions --

StatusLED::StatusLED(
    uint16_t const redPin,
    uint16_t const greenPin,
    uint16_t const bluePin,
    StatusLEDMode const mode,
    bool const show,
    SRGB const rgb,
    uint8_t const bright,
    uint32_t const delay,
    int8_t const step
):
  _redPin(redPin),
  _greenPin(greenPin),
  _bluePin(bluePin),
  _mode(mode),
  _show(show),
  _color(StatusRGB(rgb)),
  _bright(bright),
  _delay(delay),
  _step(step),
  _last(0U),
  _pulse(StatusRGB(rgb)),
  _wheel(StatusRGB(rgb)),
  _change(true),
  _chmod(true)
{
  setShow(show, true);
}

void StatusLED::update(void)
{
  static int8_t  dir = -1;
  static uint8_t pos =  0;

  if (!_show)
    { return; }

  uint32_t curr = millis();
  if (curr - _last < _delay)
    { return; }
  _last = curr;

  StatusRGB color;
  switch (_mode) {
    default:
    case StatusLEDMode::Fixed:
      color = _color;
      break;

    case StatusLEDMode::Pulse:
      if (_chmod)
        { _pulse = _color; }
      dir = nextPulse(dir, _step);
      color = _pulse;
      _change = true;
      break;

    case StatusLEDMode::Fabulous:
      if (_chmod)
        { _wheel = _color; }
      pos = nextWheel(pos, _step);
      color = _wheel;
      _change = true;
      break;
  }

  if (!_change)
    { return; }

  write(color.RGBClipped());

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
  if ((StatusLEDMode::Fixed != _mode) || (color != _color)) {
    _change = true;
    _chmod  = true;
  }
  _mode   = StatusLEDMode::Fixed;
  _color  = color;
  _bright = bright;

  setShow(true);

  update();
}

void StatusLED::pulse(
    SRGB const &rgb, uint8_t const bright, uint32_t const delay, int8_t const step)
{
  StatusRGB color = StatusRGB(rgb);
  if ((StatusLEDMode::Pulse != _mode) || (color != _color) ||
        (bright != _bright) || (delay != _delay) || (step != _step)) {
    _change = true;
    _chmod  = true;
  }
  _mode   = StatusLEDMode::Pulse;
  _color  = color;
  _delay  = delay;
  _step   = step;
  _bright = bright;

  setShow(true);

  update();
}

void StatusLED::fabulous(
    uint8_t const bright, uint32_t const delay, int8_t const step)
{
  if ((StatusLEDMode::Fabulous != _mode) ||
        (bright != _bright) || (delay != _delay) || (step != _step)) {
    _change = true;
    _chmod  = true;
  }
  _mode   = StatusLEDMode::Fabulous;
  _delay  = delay;
  _step   = step;
  _bright = bright;

  setShow(true);

  update();
}

// -------------------------------------------------------- private functions --

int16_t interp_i16(
    int16_t const x, int16_t const x1, int16_t const y1, int16_t const x2, int16_t const y2)
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
      digitalWrite(_redPin,   HIGH);
      digitalWrite(_greenPin, HIGH);
      digitalWrite(_bluePin,  HIGH);
    }
  }
}

int8_t StatusLED::nextPulse(int8_t const dir, int8_t const step)
{
#define DIMMEST(c) \
  ((step >= (c)) || ((c) <= PULSE_DIMMEST))
#define DIMMEST_COLOR(c) \
    (DIMMEST((c).red) && DIMMEST((c).green) && DIMMEST((c).blue))

#define BRIGHTEST(c) \
    ((step <= (c)) || ((c) >= PULSE_BRIGHTEST))
#define BRIGHTEST_COLOR(c) \
    (BRIGHTEST((c).red) && BRIGHTEST((c).green) && BRIGHTEST((c).blue))

  if (dir < 0) { // fade
    _pulse.red   -= step;
    _pulse.green -= step;
    _pulse.blue  -= step;
    if (DIMMEST_COLOR(_pulse))
      { return 1; }
  }
  else { // brighten
    _pulse.red   += step;
    _pulse.green += step;
    _pulse.blue  += step;
    if (BRIGHTEST_COLOR(_pulse))
      { return -1; }
  }
  return dir;

#undef DIMMEST
#undef BRIGHTEST
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

void StatusLED::write(SRGB const &rgb) const
{
  SRGB scaled = scale(rgb, _bright);
  analogWrite(_redPin,   255 - scaled.red);
  analogWrite(_greenPin, 255 - scaled.green);
  analogWrite(_bluePin,  255 - scaled.blue);
}