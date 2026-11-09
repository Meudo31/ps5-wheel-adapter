#ifndef ADAPTER_STATUS_LEDS_H
#define ADAPTER_STATUS_LEDS_H

#include <stdbool.h>
#include <stdint.h>

// Optional strip of 8 WS2812 ("NeoPixel") LEDs showing the adapter's state.
//
//   LED 0  PS5 link             LED 4  wheel
//   LED 1  USB port 1           LED 5  external Arduino brake
//   LED 2  USB port 2           LED 6  force feedback
//   LED 3  authentication       LED 7  overall summary
//
// Colour code: green = OK, blinking yellow = in progress, blinking red =
// error, off = nothing connected / not used. Exceptions are listed per LED in
// status_leds.c and in the README.

#define STATUS_LED_COUNT 8u

#ifndef STATUS_LED_BRIGHTNESS
#define STATUS_LED_BRIGHTNESS 40u  // 0..255, applied to every colour
#endif

// How long a connected port may stay without any recognised HID device
// before it is flagged (an incompatible hub, a charge-only cable...).
#define STATUS_PORT_ENUM_TIMEOUT_MS 5000u
// How long an authentication exchange may run before it counts as failed.
#define STATUS_AUTH_TIMEOUT_MS 10000u

typedef enum { STATUS_PS5_WAITING, STATUS_PS5_CONNECTED, STATUS_PS5_SUSPENDED } status_ps5_t;
typedef enum { STATUS_AUTH_ABSENT, STATUS_AUTH_IDLE, STATUS_AUTH_EXCHANGING, STATUS_AUTH_OK, STATUS_AUTH_FAILED } status_auth_t;
typedef enum {
    STATUS_WHEEL_ABSENT, STATUS_WHEEL_SETUP, STATUS_WHEEL_SWITCHING,
    STATUS_WHEEL_NATIVE, STATUS_WHEEL_COMPAT, STATUS_WHEEL_FAILED
} status_wheel_t;
typedef enum { STATUS_BRAKE_UNUSED, STATUS_BRAKE_OK, STATUS_BRAKE_FAULT } status_brake_t;

typedef struct {
    status_ps5_t ps5;
    bool port_present[2];        // port wired and enabled in firmware
    bool port_connected[2];      // something is plugged into the port
    uint32_t port_connected_ms[2];
    uint8_t port_devices[2];     // mounted HID interfaces behind the port
    status_auth_t auth;
    status_wheel_t wheel;
    status_brake_t brake;
    uint8_t ffb_profile;         // 0..4
    uint32_t ffb_sent;           // running counters
    uint32_t ffb_overflows;
} status_inputs_t;

typedef struct {
    uint32_t last_ffb_sent, last_ffb_overflows;
    uint32_t ffb_activity_ms, ffb_overflow_ms;
    bool ffb_activity_seen, ffb_overflow_seen;
} status_leds_t;

void status_leds_init(status_leds_t* leds);
// Fills out[] with one 0xRRGGBB colour per LED, brightness already applied.
void status_leds_render(status_leds_t* leds, const status_inputs_t* in, uint32_t now, uint32_t out[STATUS_LED_COUNT]);

#endif
