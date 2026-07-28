#include "semantic.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef struct mt_sha1_state {
    uint32_t h[5];
    uint64_t total_bytes;
    uint8_t block[64];
    size_t block_size;
} mt_sha1_state;

static uint32_t mt_sha1_rotl(uint32_t value, unsigned shift) {
    return (value << shift) | (value >> (32u - shift));
}

static uint32_t mt_sha1_load_be32(const uint8_t *data) {
    return ((uint32_t)data[0] << 24u) | ((uint32_t)data[1] << 16u) |
           ((uint32_t)data[2] << 8u) | (uint32_t)data[3];
}

static void mt_sha1_store_be32(uint8_t *data, uint32_t value) {
    data[0] = (uint8_t)(value >> 24u);
    data[1] = (uint8_t)(value >> 16u);
    data[2] = (uint8_t)(value >> 8u);
    data[3] = (uint8_t)value;
}

static void mt_sha1_transform(mt_sha1_state *state, const uint8_t block[64]) {
    uint32_t words[80];
    for (size_t i = 0u; i < 16u; ++i) words[i] = mt_sha1_load_be32(block + i * 4u);
    for (size_t i = 16u; i < 80u; ++i) {
        words[i] = mt_sha1_rotl(words[i - 3u] ^ words[i - 8u] ^ words[i - 14u] ^ words[i - 16u], 1u);
    }
    uint32_t a = state->h[0];
    uint32_t b = state->h[1];
    uint32_t c = state->h[2];
    uint32_t d = state->h[3];
    uint32_t e = state->h[4];
    for (size_t i = 0u; i < 80u; ++i) {
        uint32_t function;
        uint32_t constant;
        if (i < 20u) {
            function = (b & c) | ((~b) & d);
            constant = 0x5a827999u;
        } else if (i < 40u) {
            function = b ^ c ^ d;
            constant = 0x6ed9eba1u;
        } else if (i < 60u) {
            function = (b & c) | (b & d) | (c & d);
            constant = 0x8f1bbcdcu;
        } else {
            function = b ^ c ^ d;
            constant = 0xca62c1d6u;
        }
        uint32_t temporary = mt_sha1_rotl(a, 5u) + function + e + constant + words[i];
        e = d;
        d = c;
        c = mt_sha1_rotl(b, 30u);
        b = a;
        a = temporary;
    }
    state->h[0] += a;
    state->h[1] += b;
    state->h[2] += c;
    state->h[3] += d;
    state->h[4] += e;
}

void mt_sha1_init(mt_sha1_state *state) {
    if (state == NULL) return;
    state->h[0] = 0x67452301u;
    state->h[1] = 0xefcdab89u;
    state->h[2] = 0x98badcfeu;
    state->h[3] = 0x10325476u;
    state->h[4] = 0xc3d2e1f0u;
    state->total_bytes = 0u;
    state->block_size = 0u;
}

void mt_sha1_update(mt_sha1_state *state, const uint8_t *data, size_t size) {
    if (state == NULL || (data == NULL && size != 0u)) return;
    state->total_bytes += size;
    while (size != 0u) {
        size_t available = 64u - state->block_size;
        size_t amount = size < available ? size : available;
        memcpy(state->block + state->block_size, data, amount);
        state->block_size += amount;
        data += amount;
        size -= amount;
        if (state->block_size == 64u) {
            mt_sha1_transform(state, state->block);
            state->block_size = 0u;
        }
    }
}

void mt_sha1_final(mt_sha1_state *state, uint8_t digest[20]) {
    if (state == NULL || digest == NULL) return;
    uint64_t bits = state->total_bytes * UINT64_C(8);
    state->block[state->block_size++] = 0x80u;
    if (state->block_size > 56u) {
        memset(state->block + state->block_size, 0, 64u - state->block_size);
        mt_sha1_transform(state, state->block);
        state->block_size = 0u;
    }
    memset(state->block + state->block_size, 0, 56u - state->block_size);
    for (size_t i = 0u; i < 8u; ++i) state->block[63u - i] = (uint8_t)(bits >> (i * 8u));
    mt_sha1_transform(state, state->block);
    for (size_t i = 0u; i < 5u; ++i) mt_sha1_store_be32(digest + i * 4u, state->h[i]);
    memset(state, 0, sizeof(*state));
}

void mt_sha1(const uint8_t *data, size_t size, uint8_t digest[20]) {
    mt_sha1_state state;
    mt_sha1_init(&state);
    mt_sha1_update(&state, data, size);
    mt_sha1_final(&state, digest);
}
