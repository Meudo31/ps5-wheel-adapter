#ifndef ADAPTER_PEDAL_H
#define ADAPTER_PEDAL_H

#include <stdbool.h>
#include <stdint.h>

// Automatic pedal-range calibration for the wheel's own pedals.
//
// Worn G25/G27 pedals often stop short of the electrical end stops, so a
// pedal can never report fully released or fully pressed. Gran Turismo 7 can
// trim the range in its menus, but other games (Assetto Corsa Competizione,
// for example) cannot. The adapter therefore learns each pedal's travel from
// what it actually reports and stretches that travel over the full G29 range.
//
// Raw values use the Logitech convention: 255 released, 0 fully pressed.
// Output uses the G29 convention: 65535 released, 0 fully pressed.
//
// Until a pedal has been pressed to its end once, it is assumed to cover at
// least PEDAL_ASSUMED_PRESSED..PEDAL_ASSUMED_RELEASED, so the very first full
// press may saturate slightly early. The range only ever grows and is
// relearned each time the wheel connects. Build with PEDAL_AUTOCAL=0 to get
// the previous plain linear mapping.

#ifndef PEDAL_AUTOCAL
#define PEDAL_AUTOCAL 1
#endif
#ifndef PEDAL_ASSUMED_PRESSED
#define PEDAL_ASSUMED_PRESSED 40u
#endif
#ifndef PEDAL_ASSUMED_RELEASED
#define PEDAL_ASSUMED_RELEASED 215u
#endif
// Raw counts at the released end treated as fully released, so sensor noise
// at rest cannot leave a little throttle or brake applied.
#ifndef PEDAL_RELEASE_DEADZONE
#define PEDAL_RELEASE_DEADZONE 2u
#endif

typedef struct {
    uint8_t pressed;   // lowest raw value seen
    uint8_t released;  // highest raw value seen
} pedal_cal_t;

void pedal_cal_reset(pedal_cal_t* pedal);
uint16_t pedal_cal_apply(pedal_cal_t* pedal, uint8_t raw);

#endif
