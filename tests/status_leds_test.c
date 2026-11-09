#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "status_leds.h"

#define B(c) (((c) * STATUS_LED_BRIGHTNESS) / 255u)
#define RGB(r, g, b) ((B(r) << 16) | (B(g) << 8) | B(b))

static uint32_t out[STATUS_LED_COUNT];

static bool is_off(uint32_t c) { return c == 0; }
static bool is_green(uint32_t c) { return c == RGB(0, 0xff, 0); }
static bool is_red(uint32_t c) { return c == RGB(0xff, 0, 0); }
static bool is_yellow(uint32_t c) { return c == RGB(0xff, 0xa0, 0); }

int main(void) {
    status_leds_t leds;
    status_inputs_t in;
    status_leds_init(&leds);
    memset(&in, 0, sizeof(in));

    // Power-up: console not enumerated yet, nothing plugged in.
    status_leds_render(&leds, &in, 0, out);
    assert(is_yellow(out[0]) && is_off(out[1]) && is_off(out[3]) && is_off(out[4]));
    assert(is_yellow(out[7]));
    status_leds_render(&leds, &in, 250, out);  // blink phase
    assert(is_off(out[0]) && is_off(out[7]));

    // Everything connected and ready: summary turns green.
    in.ps5 = STATUS_PS5_CONNECTED;
    in.port_present[0] = in.port_present[1] = true;
    in.port_connected[0] = in.port_connected[1] = true;
    in.port_devices[0] = 1; in.port_devices[1] = 1;
    in.auth = STATUS_AUTH_OK;
    in.wheel = STATUS_WHEEL_NATIVE;
    in.ffb_profile = 1;
    status_leds_render(&leds, &in, 1000, out);
    for (unsigned i = 0; i < 5; i++) assert(is_green(out[i]));
    assert(is_off(out[5]) && is_green(out[7]));
    assert(out[6] && !is_green(out[6]));  // dim profile colour, no activity yet

    // FFB activity flashes the profile colour at full strip brightness.
    in.ffb_sent = 5;
    status_leds_render(&leds, &in, 1001, out);
    assert(is_green(out[6]));
    status_leds_render(&leds, &in, 1100, out);
    assert(!is_green(out[6]));

    // A dropped FFB command shows red for a second and flags the summary.
    in.ffb_overflows = 1;
    status_leds_render(&leds, &in, 2000, out);
    assert(is_red(out[6]) && is_red(out[7]));
    status_leds_render(&leds, &in, 3000, out);
    assert(!is_red(out[6]) && is_green(out[7]));

    // A port with something plugged in but nothing recognised: yellow, then red.
    in.port_devices[1] = 0; in.port_connected_ms[1] = 4000;
    status_leds_render(&leds, &in, 4000, out);
    assert(is_yellow(out[2]) && is_yellow(out[7]));
    status_leds_render(&leds, &in, 4000 + STATUS_PORT_ENUM_TIMEOUT_MS, out);
    assert(is_red(out[2]) && is_red(out[7]));
    in.port_devices[1] = 1;

    // Authentication states.
    in.auth = STATUS_AUTH_EXCHANGING;
    status_leds_render(&leds, &in, 10000, out);
    assert(is_yellow(out[3]) && is_yellow(out[7]));
    in.auth = STATUS_AUTH_FAILED;
    status_leds_render(&leds, &in, 10000, out);
    assert(is_red(out[3]) && is_red(out[7]));
    in.auth = STATUS_AUTH_OK;

    // Wheel: compatibility mode is cyan; failure is red; FFB LED off when not ready.
    in.wheel = STATUS_WHEEL_COMPAT;
    status_leds_render(&leds, &in, 10000, out);
    assert(out[4] == RGB(0, 0xff, 0xff) && is_green(out[7]));
    in.wheel = STATUS_WHEEL_FAILED;
    status_leds_render(&leds, &in, 10000, out);
    assert(is_red(out[4]) && is_off(out[6]) && is_red(out[7]));
    in.wheel = STATUS_WHEEL_NATIVE;

    // Brake fault.
    in.brake = STATUS_BRAKE_FAULT;
    status_leds_render(&leds, &in, 10000, out);
    assert(is_red(out[5]) && is_red(out[7]));
    in.brake = STATUS_BRAKE_OK;
    status_leds_render(&leds, &in, 10000, out);
    assert(is_green(out[5]) && is_green(out[7]));

    // Console in rest mode.
    in.ps5 = STATUS_PS5_SUSPENDED;
    status_leds_render(&leds, &in, 10000, out);
    assert(out[0] == RGB(0, 0, 0xff));

    puts("Status LEDs passed.");
    return 0;
}
