#include "ws2812.h"

#include "hardware/pio.h"
#include "ws2812.pio.h"

static PIO ws_pio;
static int ws_sm = -1;

bool ws2812_init(void) {
    // Pico-PIO-USB has already claimed its state machines and loaded its
    // programs, so only genuinely free resources are taken here.
    PIO candidates[2] = { pio1, pio0 };
    for (unsigned i = 0; i < 2; i++) {
        PIO pio = candidates[i];
        if (!pio_can_add_program(pio, &ws2812_program)) continue;
        int sm = pio_claim_unused_sm(pio, false);
        if (sm < 0) continue;
        uint offset = pio_add_program(pio, &ws2812_program);
        ws2812_program_init(pio, (uint) sm, offset, STATUS_LED_PIN, 800000.0f);
        ws_pio = pio;
        ws_sm = sm;
        return true;
    }
    return false;
}

bool ws2812_show(const uint32_t* rgb, unsigned count) {
    if (ws_sm < 0) return false;
    if (!pio_sm_is_tx_fifo_empty(ws_pio, (uint) ws_sm)) return false;
    for (unsigned i = 0; i < count; i++) {
        uint32_t r = (rgb[i] >> 16) & 0xffu, g = (rgb[i] >> 8) & 0xffu, b = rgb[i] & 0xffu;
        // WS2812 expects GRB, most significant bit first.
        pio_sm_put(ws_pio, (uint) ws_sm, ((g << 16) | (r << 8) | b) << 8u);
    }
    return true;
}
