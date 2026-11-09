#include "auth_detect.h"

#include <stddef.h>

#define HID_ITEM_LONG 0xfeu
#define HID_TAG_FEATURE 0xb0u      // Main item, tag 0xB
#define HID_TAG_REPORT_ID 0x84u    // Global item, tag 0x8
#define HID_TAG_PUSH 0xa4u         // Global item, tag 0xA
#define HID_TAG_POP 0xb4u          // Global item, tag 0xB
#define HID_STACK_DEPTH 8u

bool hid_descriptor_has_feature(const uint8_t* desc, uint16_t len, uint8_t report_id) {
    if (!desc) return false;
    uint8_t current_id = 0;
    uint8_t stack[HID_STACK_DEPTH];
    uint8_t depth = 0;
    size_t i = 0;
    while (i < len) {
        uint8_t prefix = desc[i];
        if (prefix == HID_ITEM_LONG) {
            // Long item: prefix, data size, tag, data.
            if (i + 1u >= len) return false;
            i += 3u + desc[i + 1u];
            continue;
        }
        uint8_t size = prefix & 0x03u;
        if (size == 3u) size = 4u;
        if (i + 1u + size > len) return false;  // truncated descriptor
        uint8_t tag = prefix & 0xfcu;
        if (tag == HID_TAG_REPORT_ID && size >= 1u) {
            current_id = desc[i + 1u];
        } else if (tag == HID_TAG_PUSH) {
            if (depth < HID_STACK_DEPTH) stack[depth] = current_id;
            depth++;
        } else if (tag == HID_TAG_POP) {
            if (depth) {
                depth--;
                if (depth < HID_STACK_DEPTH) current_id = stack[depth];
            }
        } else if (tag == HID_TAG_FEATURE && current_id == report_id) {
            return true;
        }
        i += 1u + size;
    }
    return false;
}

auth_rank_t auth_rank_from_descriptor(const uint8_t* desc, uint16_t len) {
    if (!desc || !len) return AUTH_RANK_UNKNOWN;
    // The console's nonce goes out as 0xF0, the signature comes back as 0xF1,
    // 0xF2 reports signing progress and 0xF3 resets the exchange. Some
    // licensed controllers declare only part of the set, so any one counts.
    for (unsigned id = 0xf0u; id <= 0xf3u; id++) {
        if (hid_descriptor_has_feature(desc, len, (uint8_t) id)) return AUTH_RANK_CONFIRMED;
    }
    return AUTH_RANK_NO_AUTH_REPORTS;
}

const char* auth_rank_name(auth_rank_t rank) {
    switch (rank) {
        case AUTH_RANK_CONFIRMED: return "declares PS4 auth reports";
        case AUTH_RANK_UNKNOWN: return "descriptor unavailable";
        case AUTH_RANK_NO_AUTH_REPORTS: return "no auth reports, fallback";
        default: return "none";
    }
}
