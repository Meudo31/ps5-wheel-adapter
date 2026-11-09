#include <assert.h>
#include <stdio.h>
#include "auth_detect.h"

// Tail of the descriptor the adapter itself presents (the same vendor page
// and report IDs a DualShock 4 and licensed PS4 controllers use).
static const uint8_t auth_collection[] = {
    0x06, 0xF0, 0xFF, 0x09, 0x40, 0xA1, 0x01,
    0x85, 0xF0, 0x09, 0x47, 0x95, 0x3F, 0xB1, 0x02,
    0x85, 0xF1, 0x09, 0x48, 0x95, 0x3F, 0xB1, 0x02,
    0x85, 0xF2, 0x09, 0x49, 0x95, 0x0F, 0xB1, 0x02,
    0x85, 0xF3, 0x0A, 0x01, 0x47, 0x95, 0x07, 0xB1, 0x02,
    0xC0,
};

// A plain gamepad: input report 1 and output report 5, no feature reports.
static const uint8_t gamepad[] = {
    0x05, 0x01, 0x09, 0x05, 0xA1, 0x01, 0x85, 0x01,
    0x09, 0x30, 0x15, 0x00, 0x26, 0xFF, 0x00, 0x75, 0x08, 0x95, 0x01, 0x81, 0x02,
    0x85, 0x05, 0x09, 0x22, 0x95, 0x1F, 0x91, 0x02,
    0xC0,
};

// Report ID F0 used for an Input item, and Feature items on another ID.
static const uint8_t decoy[] = {
    0x85, 0xF0, 0x09, 0x01, 0x81, 0x02,
    0x85, 0x02, 0x09, 0x02, 0xB1, 0x02,
};

// Push/Pop restores the report ID in effect.
static const uint8_t push_pop[] = {
    0x85, 0x02, 0xA4, 0x85, 0xF1, 0xB4, 0x09, 0x01, 0xB1, 0x02,
};

int main(void) {
    assert(hid_descriptor_has_feature(auth_collection, sizeof(auth_collection), 0xF0));
    assert(hid_descriptor_has_feature(auth_collection, sizeof(auth_collection), 0xF3));
    assert(auth_rank_from_descriptor(auth_collection, sizeof(auth_collection)) == AUTH_RANK_CONFIRMED);

    assert(!hid_descriptor_has_feature(gamepad, sizeof(gamepad), 0xF0));
    assert(auth_rank_from_descriptor(gamepad, sizeof(gamepad)) == AUTH_RANK_NO_AUTH_REPORTS);

    assert(auth_rank_from_descriptor(decoy, sizeof(decoy)) == AUTH_RANK_NO_AUTH_REPORTS);
    assert(hid_descriptor_has_feature(decoy, sizeof(decoy), 0x02));

    assert(!hid_descriptor_has_feature(push_pop, sizeof(push_pop), 0xF1));
    assert(hid_descriptor_has_feature(push_pop, sizeof(push_pop), 0x02));

    // Missing or truncated descriptors never crash and stay usable as fallback.
    assert(auth_rank_from_descriptor(NULL, 0) == AUTH_RANK_UNKNOWN);
    assert(!hid_descriptor_has_feature(auth_collection, 8, 0xF0));
    const uint8_t truncated_long[] = { 0xFE, 0x40 };
    assert(!hid_descriptor_has_feature(truncated_long, sizeof(truncated_long), 0xF0));

    assert(AUTH_RANK_CONFIRMED > AUTH_RANK_UNKNOWN && AUTH_RANK_UNKNOWN > AUTH_RANK_NO_AUTH_REPORTS);
    puts("Authentication controller detection passed.");
    return 0;
}
