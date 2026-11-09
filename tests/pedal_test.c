#include <assert.h>
#include <stdio.h>
#include "pedal.h"

int main(void) {
    pedal_cal_t p;

    // Healthy pedal: once both ends have been seen, the mapping is full range.
    pedal_cal_reset(&p);
    assert(pedal_cal_apply(&p, 255) == UINT16_MAX);
    assert(pedal_cal_apply(&p, 0) == 0);
    assert(pedal_cal_apply(&p, 253) == UINT16_MAX);  // release deadzone
    assert(pedal_cal_apply(&p, 252) < UINT16_MAX);
    uint16_t mid = pedal_cal_apply(&p, 126);
    assert(mid > 32000 && mid < 33500);

    // Monotonic: more pressure never gives a higher (more released) value.
    uint16_t previous = UINT16_MAX;
    for (int raw = 255; raw >= 0; raw--) {
        uint16_t value = pedal_cal_apply(&p, (uint8_t) raw);
        assert(value <= previous);
        previous = value;
    }

    // Worn pedal: rests at 240 and bottoms out at 25. Both ends still map to
    // the full G29 range after one press.
    pedal_cal_reset(&p);
    assert(pedal_cal_apply(&p, 240) == UINT16_MAX);
    assert(pedal_cal_apply(&p, 25) == 0);
    assert(pedal_cal_apply(&p, 238) == UINT16_MAX);
    assert(pedal_cal_apply(&p, 237) < UINT16_MAX);
    assert(pedal_cal_apply(&p, 26) > 0);

    // Before the first full press, the assumed window saturates early.
    pedal_cal_reset(&p);
    assert(pedal_cal_apply(&p, PEDAL_ASSUMED_PRESSED) == 0);
    assert(pedal_cal_apply(&p, PEDAL_ASSUMED_PRESSED + 1u) > 0);

    puts("Pedal calibration passed.");
    return 0;
}
