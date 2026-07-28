#include "semantic.h"

#include <stddef.h>
#include <stdint.h>

static uint32_t mt_crc_reflect(uint32_t value, unsigned bits) {
    uint32_t result = 0u;
    for (unsigned i = 0u; i < bits; ++i) {
        result = (result << 1u) | (value & 1u);
        value >>= 1u;
    }
    return result;
}

uint32_t mt_crc32_update(uint32_t state, const uint8_t *data, size_t size) {
    if (data == NULL && size != 0u) return state;
    uint32_t crc = state;
    for (size_t i = 0u; i < size; ++i) {
        crc ^= data[i];
        for (unsigned bit = 0u; bit < 8u; ++bit) {
            uint32_t mask = 0u - (crc & 1u);
            crc = (crc >> 1u) ^ (0xedb88320u & mask);
        }
    }
    return crc;
}

uint32_t mt_crc32(const uint8_t *data, size_t size) {
    return mt_crc32_update(0xffffffffu, data, size) ^ 0xffffffffu;
}

uint32_t mt_crc32c_update(uint32_t state, const uint8_t *data, size_t size) {
    if (data == NULL && size != 0u) return state;
    uint32_t crc = state;
    for (size_t i = 0u; i < size; ++i) {
        crc ^= data[i];
        for (unsigned bit = 0u; bit < 8u; ++bit) {
            uint32_t mask = 0u - (crc & 1u);
            crc = (crc >> 1u) ^ (0x82f63b78u & mask);
        }
    }
    return crc;
}

uint32_t mt_crc32c(const uint8_t *data, size_t size) {
    return mt_crc32c_update(0xffffffffu, data, size) ^ 0xffffffffu;
}

uint16_t mt_crc16_ccitt_update(uint16_t state, const uint8_t *data, size_t size) {
    if (data == NULL && size != 0u) return state;
    uint16_t crc = state;
    for (size_t i = 0u; i < size; ++i) {
        crc ^= (uint16_t)data[i] << 8u;
        for (unsigned bit = 0u; bit < 8u; ++bit) {
            crc = (uint16_t)((crc << 1u) ^ ((crc & 0x8000u) != 0u ? 0x1021u : 0u));
        }
    }
    return crc;
}

uint16_t mt_crc16_ccitt(const uint8_t *data, size_t size) {
    return mt_crc16_ccitt_update(0xffffu, data, size);
}

uint64_t mt_crc64_ecma_update(uint64_t state, const uint8_t *data, size_t size) {
    if (data == NULL && size != 0u) return state;
    uint64_t crc = state;
    for (size_t i = 0u; i < size; ++i) {
        crc ^= (uint64_t)data[i] << 56u;
        for (unsigned bit = 0u; bit < 8u; ++bit) {
            crc = (crc << 1u) ^ ((crc & UINT64_C(0x8000000000000000)) != 0u
                                ? UINT64_C(0x42f0e1eba9ea3693) : UINT64_C(0));
        }
    }
    return crc;
}

uint64_t mt_crc64_ecma(const uint8_t *data, size_t size) {
    return mt_crc64_ecma_update(UINT64_C(0), data, size);
}

uint32_t mt_crc_generic(const uint8_t *data, size_t size, unsigned width,
                        uint32_t polynomial, uint32_t initial,
                        uint32_t final_xor, bool reflect_input,
                        bool reflect_output) {
    if (width == 0u || width > 32u || (data == NULL && size != 0u)) return 0u;
    uint32_t top = UINT32_C(1) << (width - 1u);
    uint32_t mask = width == 32u ? UINT32_MAX : (UINT32_C(1) << width) - 1u;
    uint32_t crc = initial & mask;
    for (size_t i = 0u; i < size; ++i) {
        uint32_t byte = data[i];
        if (reflect_input) byte = mt_crc_reflect(byte, 8u);
        crc ^= byte << (width >= 8u ? width - 8u : 0u);
        for (unsigned bit = 0u; bit < 8u; ++bit) {
            bool set = (crc & top) != 0u;
            crc = (crc << 1u) & mask;
            if (set) crc ^= polynomial;
        }
    }
    if (reflect_output) crc = mt_crc_reflect(crc, width);
    return (crc ^ final_xor) & mask;
}
