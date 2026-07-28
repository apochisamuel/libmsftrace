#include "semantic.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static bool mt_utf8_continuation(uint8_t byte) { return (byte & 0xc0u) == 0x80u; }

bool mt_utf8_validate(const uint8_t *data, size_t size) {
    if (data == NULL && size != 0u) return false;
    size_t pos = 0u;
    while (pos < size) {
        uint8_t first = data[pos++];
        if (first < 0x80u) continue;
        if (first >= 0xc2u && first <= 0xdfu) {
            if (pos >= size || !mt_utf8_continuation(data[pos])) return false;
            ++pos;
            continue;
        }
        if (first >= 0xe0u && first <= 0xefu) {
            if (pos + 1u >= size) return false;
            uint8_t second = data[pos];
            uint8_t third = data[pos + 1u];
            if (!mt_utf8_continuation(second) || !mt_utf8_continuation(third)) return false;
            if (first == 0xe0u && second < 0xa0u) return false;
            if (first == 0xedu && second >= 0xa0u) return false;
            pos += 2u;
            continue;
        }
        if (first >= 0xf0u && first <= 0xf4u) {
            if (pos + 2u >= size) return false;
            uint8_t second = data[pos];
            uint8_t third = data[pos + 1u];
            uint8_t fourth = data[pos + 2u];
            if (!mt_utf8_continuation(second) || !mt_utf8_continuation(third) ||
                !mt_utf8_continuation(fourth)) return false;
            if (first == 0xf0u && second < 0x90u) return false;
            if (first == 0xf4u && second >= 0x90u) return false;
            pos += 3u;
            continue;
        }
        return false;
    }
    return true;
}

size_t mt_utf8_count_codepoints(const uint8_t *data, size_t size) {
    if (!mt_utf8_validate(data, size)) return SIZE_MAX;
    size_t count = 0u;
    for (size_t i = 0u; i < size; ++i) {
        if ((data[i] & 0xc0u) != 0x80u) ++count;
    }
    return count;
}

size_t mt_utf8_encode(uint32_t codepoint, uint8_t output[4]) {
    if (output == NULL) return 0u;
    if (codepoint <= 0x7fu) {
        output[0] = (uint8_t)codepoint;
        return 1u;
    }
    if (codepoint <= 0x7ffu) {
        output[0] = (uint8_t)(0xc0u | (codepoint >> 6u));
        output[1] = (uint8_t)(0x80u | (codepoint & 0x3fu));
        return 2u;
    }
    if (codepoint >= 0xd800u && codepoint <= 0xdfffu) return 0u;
    if (codepoint <= 0xffffu) {
        output[0] = (uint8_t)(0xe0u | (codepoint >> 12u));
        output[1] = (uint8_t)(0x80u | ((codepoint >> 6u) & 0x3fu));
        output[2] = (uint8_t)(0x80u | (codepoint & 0x3fu));
        return 3u;
    }
    if (codepoint <= 0x10ffffu) {
        output[0] = (uint8_t)(0xf0u | (codepoint >> 18u));
        output[1] = (uint8_t)(0x80u | ((codepoint >> 12u) & 0x3fu));
        output[2] = (uint8_t)(0x80u | ((codepoint >> 6u) & 0x3fu));
        output[3] = (uint8_t)(0x80u | (codepoint & 0x3fu));
        return 4u;
    }
    return 0u;
}
