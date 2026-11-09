#include "status_leds.h"

#include <string.h>

#define OFF     0x000000u
#define GREEN   0x00ff00u
#define YELLOW  0xffa000u
#define RED     0xff0000u
#define BLUE    0x0000ffu
#define CYAN    0x00ffffu
#define WHITE   0xffffffu
#define MAGENTA 0xff00ffu

#define BLINK_MS 250u
#define FFB_ACTIVITY_MS 60u
#define FFB_OVERFLOW_MS 1000u

static uint32_t scale(uint32_t rgb, uint32_t level) {
    uint32_t r = ((rgb >> 16) & 0xffu) * level / 255u;
    uint32_t g = ((rgb >> 8) & 0xffu) * level / 255u;
    uint32_t b = (rgb & 0xffu) * level / 255u;
    return (r << 16) | (g << 8) | b;
}

static uint32_t blink(uint32_t rgb, uint32_t now) {
    return (now / BLINK_MS) & 1u ? OFF : rgb;
}

static bool recent(bool seen, uint32_t at, uint32_t now, uint32_t window) {
    return seen && (uint32_t) (now - at) < window;
}

void status_leds_init(status_leds_t* leds) {
    memset(leds, 0, sizeof(*leds));
}

// Port LED: off = empty, blinking yellow = enumerating, green = at least one
// HID device recognised, blinking red = connected but nothing usable after
// STATUS_PORT_ENUM_TIMEOUT_MS (incompatible hub, power or cable problem).
static uint32_t port_colour(const status_inputs_t* in, unsigned port, uint32_t now, bool* error, bool* busy) {
    if (!in->port_present[port] || !in->port_connected[port]) return OFF;
    if (in->port_devices[port]) return GREEN;
    if ((uint32_t) (now - in->port_connected_ms[port]) < STATUS_PORT_ENUM_TIMEOUT_MS) {
        *busy = true;
        return blink(YELLOW, now);
    }
    *error = true;
    return blink(RED, now);
}

void status_leds_render(status_leds_t* leds, const status_inputs_t* in, uint32_t now, uint32_t out[STATUS_LED_COUNT]) {
    bool error = false, busy = false;

    // FFB counters only ever grow; note when they move.
    if (in->ffb_sent != leds->last_ffb_sent) {
        leds->last_ffb_sent = in->ffb_sent;
        leds->ffb_activity_ms = now;
        leds->ffb_activity_seen = true;
    }
    if (in->ffb_overflows != leds->last_ffb_overflows) {
        leds->last_ffb_overflows = in->ffb_overflows;
        leds->ffb_overflow_ms = now;
        leds->ffb_overflow_seen = true;
    }

    // 0: PS5. Blue = console in rest mode (USB suspended).
    switch (in->ps5) {
        case STATUS_PS5_CONNECTED: out[0] = GREEN; break;
        case STATUS_PS5_SUSPENDED: out[0] = BLUE; break;
        default: out[0] = blink(YELLOW, now); busy = true; break;
    }

    // 1, 2: downstream USB ports.
    out[1] = port_colour(in, 0, now, &error, &busy);
    out[2] = port_colour(in, 1, now, &error, &busy);

    // 3: authentication. Steady yellow = controller present, console has not
    // asked yet; blinking yellow = signing in progress.
    switch (in->auth) {
        case STATUS_AUTH_IDLE: out[3] = YELLOW; break;
        case STATUS_AUTH_EXCHANGING: out[3] = blink(YELLOW, now); busy = true; break;
        case STATUS_AUTH_OK: out[3] = GREEN; break;
        case STATUS_AUTH_FAILED: out[3] = blink(RED, now); error = true; break;
        default: out[3] = OFF; break;
    }

    // 4: wheel. Cyan = ready in Driving Force compatibility mode (no clutch,
    // H-shifter or extra buttons).
    switch (in->wheel) {
        case STATUS_WHEEL_SETUP:
        case STATUS_WHEEL_SWITCHING: out[4] = blink(YELLOW, now); busy = true; break;
        case STATUS_WHEEL_NATIVE: out[4] = GREEN; break;
        case STATUS_WHEEL_COMPAT: out[4] = CYAN; break;
        case STATUS_WHEEL_FAILED: out[4] = blink(RED, now); error = true; break;
        default: out[4] = OFF; break;
    }

    // 5: external brake.
    switch (in->brake) {
        case STATUS_BRAKE_OK: out[5] = GREEN; break;
        case STATUS_BRAKE_FAULT: out[5] = blink(RED, now); error = true; break;
        default: out[5] = OFF; break;
    }

    // 6: force feedback. Dim colour = selected profile, brief full flash on
    // each command sent to the wheel, one second of red after a dropped one.
    static const uint32_t profile_colours[5] = { WHITE, GREEN, YELLOW, MAGENTA, CYAN };
    bool wheel_ready = in->wheel == STATUS_WHEEL_NATIVE || in->wheel == STATUS_WHEEL_COMPAT;
    if (recent(leds->ffb_overflow_seen, leds->ffb_overflow_ms, now, FFB_OVERFLOW_MS)) {
        out[6] = RED;
        error = true;
    } else if (!wheel_ready) {
        out[6] = OFF;
    } else {
        uint32_t colour = profile_colours[in->ffb_profile < 5u ? in->ffb_profile : 0u];
        out[6] = recent(leds->ffb_activity_seen, leds->ffb_activity_ms, now, FFB_ACTIVITY_MS) ?
            colour : scale(colour, 64u);
    }

    // 7: summary. Red = any error, yellow = something still connecting,
    // green = console, authentication and wheel all ready.
    if (error) out[7] = blink(RED, now);
    else if (busy) out[7] = blink(YELLOW, now);
    else if (in->ps5 == STATUS_PS5_CONNECTED && in->auth == STATUS_AUTH_OK && wheel_ready) out[7] = GREEN;
    else out[7] = YELLOW;

    for (unsigned i = 0; i < STATUS_LED_COUNT; i++) out[i] = scale(out[i], STATUS_LED_BRIGHTNESS);
}
