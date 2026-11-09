#ifndef ADAPTER_WS2812_H
#define ADAPTER_WS2812_H

#include <stdbool.h>
#include <stdint.h>

#ifndef STATUS_LED_PIN
#define STATUS_LED_PIN 6
#endif

// Claims a free PIO state machine left over by the USB host. Returns false
// (and the strip stays dark) when none is free; the adapter works regardless.
bool ws2812_init(void);
// Queues one frame without blocking. Returns false if the previous frame is
// still being shifted out.
bool ws2812_show(const uint32_t* rgb, unsigned count);

#endif
