#include <Arduino.h>
#line 1 "/opt/arduino/src/libraries/StatusLED/examples/Blinky/Blinky.ino"
// Blinky.ino (automatically generated 2023-01-15 12:05:55)

#include <StatusLED.h>

// Return type of Arduino core function millis()
typedef unsigned long duration_t;

// Turn LED on for ONDUTY ms every PERIOD ms
static duration_t const PERIOD = 1000UL; // 1.0 s
static duration_t const ONDUTY =  300UL; // 0.3 s

// Target device wiring configuration
#define PIN_LED_RED   3
#define PIN_LED_GREEN 5
#define PIN_LED_BLUE  6
//#define PIN_LED_WHITE 9 // No white channel on 4-pin 9W common anode RGB LED

#define LED_ON  HIGH
#define LED_OFF LOW

StatusLED *led;

#line 23 "/opt/arduino/src/libraries/StatusLED/examples/Blinky/Blinky.ino"
void setup();
#line 29 "/opt/arduino/src/libraries/StatusLED/examples/Blinky/Blinky.ino"
void loop();
#line 23 "/opt/arduino/src/libraries/StatusLED/examples/Blinky/Blinky.ino"
void setup() {
  led = new StatusLED(
    PIN_LED_RED, PIN_LED_GREEN, PIN_LED_BLUE, LED_OFF, StatusLEDMode::Fabulous,
    10UL, true, SRGB{});
}

void loop() {
  led->update();
}

