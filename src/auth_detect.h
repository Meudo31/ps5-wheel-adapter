#ifndef ADAPTER_AUTH_DETECT_H
#define ADAPTER_AUTH_DETECT_H

#include <stdbool.h>
#include <stdint.h>

// Ranking of HID interfaces as PS4 authentication controller candidates.
// A higher rank always replaces a lower one; an equal or lower rank never
// replaces the current candidate.
typedef enum {
    AUTH_RANK_NONE = 0,
    // The descriptor was read and declares none of the authentication reports.
    AUTH_RANK_NO_AUTH_REPORTS,
    // No descriptor available (TinyUSB skips descriptors over 512 bytes):
    // still usable, as in the original firmware.
    AUTH_RANK_UNKNOWN,
    // Declares the PS4 authentication feature reports 0xF0..0xF3.
    AUTH_RANK_CONFIRMED,
} auth_rank_t;

// True when a HID report descriptor declares a Feature item with this report ID.
bool hid_descriptor_has_feature(const uint8_t* desc, uint16_t len, uint8_t report_id);
auth_rank_t auth_rank_from_descriptor(const uint8_t* desc, uint16_t len);
const char* auth_rank_name(auth_rank_t rank);

#endif
