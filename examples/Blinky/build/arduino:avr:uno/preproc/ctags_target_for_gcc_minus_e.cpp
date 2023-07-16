# 1 "/opt/arduino/src/libraries/StatusLED/examples/Blinky/Blinky.ino"
// Blinky.ino (automatically generated 2023-01-15 12:05:55)

# 4 "/opt/arduino/src/libraries/StatusLED/examples/Blinky/Blinky.ino" 2

// Return type of Arduino core function millis()
typedef unsigned long duration_t;

// Turn LED on for ONDUTY ms every PERIOD ms
static duration_t const PERIOD = 1000UL; // 1.0 s
static duration_t const ONDUTY = 300UL; // 0.3 s

// Target device wiring configuration



//#define PIN_LED_WHITE 9 // No white channel on 4-pin 9W common anode RGB LED




StatusLED *led;

void setup() {
  led = new StatusLED(
    3, 5, 6, 0x0, StatusLEDMode::Fabulous,
    10UL, true, SRGB{});
}

void loop() {
  led->update();
}
