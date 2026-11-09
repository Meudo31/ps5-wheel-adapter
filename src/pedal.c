#include "pedal.h"

_Static_assert(PEDAL_ASSUMED_PRESSED < PEDAL_ASSUMED_RELEASED, "Assumed pedal window must not be empty");
_Static_assert(PEDAL_ASSUMED_RELEASED - PEDAL_ASSUMED_PRESSED > PEDAL_RELEASE_DEADZONE,
    "Release deadzone must leave usable travel");

void pedal_cal_reset(pedal_cal_t* pedal) {
    pedal->pressed = PEDAL_ASSUMED_PRESSED;
    pedal->released = PEDAL_ASSUMED_RELEASED;
}

uint16_t pedal_cal_apply(pedal_cal_t* pedal, uint8_t raw) {
#if PEDAL_AUTOCAL
    if (raw < pedal->pressed) pedal->pressed = raw;
    if (raw > pedal->released) pedal->released = raw;

    uint32_t top = (uint32_t) pedal->released - PEDAL_RELEASE_DEADZONE;
    if (raw >= top) return UINT16_MAX;
    if (raw <= pedal->pressed) return 0;
    uint32_t span = top - pedal->pressed;
    return (uint16_t) (((uint32_t) (raw - pedal->pressed) * UINT16_MAX + span / 2u) / span);
#else
    (void) pedal;
    return (uint16_t) raw * 257u;
#endif
}
