#include "semantic.h"

#include <ctype.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int mt_hex_value(char ch) {
    if (ch >= '0' && ch <= '9') return ch - '0';
    ch = (char)tolower((unsigned char)ch);
    if (ch >= 'a' && ch <= 'f') return 10 + ch - 'a';
    return -1;
}

mt_status mt_guid_parse(const char *text, uint8_t output[16]) {
    if (text == NULL || output == NULL) return MT_ERR_ARGUMENT;
    static const size_t dash_positions[] = {8u, 13u, 18u, 23u};
    size_t dash_index = 0u;
    size_t byte_index = 0u;
    size_t pos = 0u;
    while (text[pos] != '\0') {
        if (dash_index < sizeof(dash_positions) / sizeof(dash_positions[0]) &&
            pos == dash_positions[dash_index]) {
            if (text[pos] != '-') return MT_ERR_FORMAT;
            ++dash_index;
            ++pos;
            continue;
        }
        if (byte_index >= 16u || text[pos + 1u] == '\0') return MT_ERR_FORMAT;
        int high = mt_hex_value(text[pos]);
        int low = mt_hex_value(text[pos + 1u]);
        if (high < 0 || low < 0) return MT_ERR_FORMAT;
        output[byte_index++] = (uint8_t)((high << 4) | low);
        pos += 2u;
    }
    return byte_index == 16u && pos == 36u ? MT_OK : MT_ERR_FORMAT;
}

mt_status mt_guid_format(const uint8_t guid[16], char output[37]) {
    if (guid == NULL || output == NULL) return MT_ERR_ARGUMENT;
    int length = snprintf(output, 37u,
                          "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                          guid[0], guid[1], guid[2], guid[3], guid[4], guid[5],
                          guid[6], guid[7], guid[8], guid[9], guid[10], guid[11],
                          guid[12], guid[13], guid[14], guid[15]);
    return length == 36 ? MT_OK : MT_ERR_FORMAT;
}

bool mt_guid_equal(const uint8_t left[16], const uint8_t right[16]) {
    return left != NULL && right != NULL && memcmp(left, right, 16u) == 0;
}

uint32_t mt_guid_hash(const uint8_t guid[16], uint32_t seed) {
    return guid == NULL ? 0u : mt_hash_bytes(guid, 16u, seed);
}
