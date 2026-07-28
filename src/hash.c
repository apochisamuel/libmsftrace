#include "semantic.h"

#include <ctype.h>
#include <string.h>

static uint32_t mt_rotate_left32(uint32_t value, unsigned shift) {
    shift &= 31u;
    return shift == 0u ? value : (value << shift) | (value >> (32u - shift));
}

uint32_t mt_hash_bytes(const uint8_t *data, size_t size, uint32_t seed) {
    uint32_t a = 0x9e3779b9u ^ seed;
    uint32_t b = 0x85ebca6bu + (uint32_t)size;
    uint32_t c = 0xc2b2ae35u ^ (uint32_t)(size >> 32u);
    if (data == NULL && size != 0u) return 0u;
    size_t pos = 0u;
    while (pos + 12u <= size) {
        uint32_t x = (uint32_t)data[pos] |
                     ((uint32_t)data[pos + 1u] << 8u) |
                     ((uint32_t)data[pos + 2u] << 16u) |
                     ((uint32_t)data[pos + 3u] << 24u);
        uint32_t y = (uint32_t)data[pos + 4u] |
                     ((uint32_t)data[pos + 5u] << 8u) |
                     ((uint32_t)data[pos + 6u] << 16u) |
                     ((uint32_t)data[pos + 7u] << 24u);
        uint32_t z = (uint32_t)data[pos + 8u] |
                     ((uint32_t)data[pos + 9u] << 8u) |
                     ((uint32_t)data[pos + 10u] << 16u) |
                     ((uint32_t)data[pos + 11u] << 24u);
        a += x;
        b += y;
        c += z;
        a -= c; a ^= mt_rotate_left32(c, 4u); c += b;
        b -= a; b ^= mt_rotate_left32(a, 6u); a += c;
        c -= b; c ^= mt_rotate_left32(b, 8u); b += a;
        a -= c; a ^= mt_rotate_left32(c, 16u); c += b;
        b -= a; b ^= mt_rotate_left32(a, 19u); a += c;
        c -= b; c ^= mt_rotate_left32(b, 4u); b += a;
        pos += 12u;
    }
    uint32_t tail_a = 0u;
    uint32_t tail_b = 0u;
    uint32_t tail_c = 0u;
    size_t remain = size - pos;
    for (size_t i = 0u; i < remain; ++i) {
        uint32_t byte = data[pos + i];
        if (i < 4u) tail_a |= byte << (8u * i);
        else if (i < 8u) tail_b |= byte << (8u * (i - 4u));
        else tail_c |= byte << (8u * (i - 8u));
    }
    a += tail_a;
    b += tail_b;
    c += tail_c;
    c ^= b; c -= mt_rotate_left32(b, 14u);
    a ^= c; a -= mt_rotate_left32(c, 11u);
    b ^= a; b -= mt_rotate_left32(a, 25u);
    c ^= b; c -= mt_rotate_left32(b, 16u);
    a ^= c; a -= mt_rotate_left32(c, 4u);
    b ^= a; b -= mt_rotate_left32(a, 14u);
    c ^= b; c -= mt_rotate_left32(b, 24u);
    return c;
}

uint32_t mt_hash_string_ci(const char *text, uint32_t seed) {
    if (text == NULL) return mt_hash_bytes(NULL, 0u, seed);
    uint32_t hash = 2166136261u ^ seed;
    for (const unsigned char *p = (const unsigned char *)text; *p != 0u; ++p) {
        unsigned char folded = (unsigned char)tolower(*p);
        hash ^= folded;
        hash *= 16777619u;
        hash ^= hash >> 13u;
    }
    hash ^= hash >> 16u;
    hash *= 0x7feb352du;
    hash ^= hash >> 15u;
    hash *= 0x846ca68bu;
    hash ^= hash >> 16u;
    return hash;
}

bool mt_string_equal_ci(const char *left, const char *right) {
    if (left == right) return true;
    if (left == NULL || right == NULL) return false;
    while (*left != '\0' && *right != '\0') {
        unsigned char a = (unsigned char)tolower((unsigned char)*left);
        unsigned char b = (unsigned char)tolower((unsigned char)*right);
        if (a != b) return false;
        ++left;
        ++right;
    }
    return *left == *right;
}

size_t mt_normalize_path(const char *path, char *output, size_t capacity) {
    if (path == NULL) {
        if (output != NULL && capacity != 0u) output[0] = '\0';
        return 0u;
    }
    size_t write = 0u;
    bool last_separator = false;
    const char *cursor = path;
    while (*cursor != '\0') {
        unsigned char ch = (unsigned char)*cursor++;
        bool separator = ch == '/' || ch == '\\';
        if (separator) {
            if (last_separator) continue;
            ch = '/';
        } else {
            ch = (unsigned char)tolower(ch);
        }
        if (output != NULL && write + 1u < capacity) output[write] = (char)ch;
        ++write;
        last_separator = separator;
    }
    while (write > 1u) {
        char previous = output != NULL && write - 1u < capacity ? output[write - 1u] : path[write - 1u];
        if (previous != '/') break;
        --write;
    }
    if (output != NULL && capacity != 0u) {
        size_t end = write < capacity - 1u ? write : capacity - 1u;
        output[end] = '\0';
    }
    return write;
}
