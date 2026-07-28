#include "semantic.h"

#include <stddef.h>
#include <stdint.h>

mt_status mt_varint_decode_u32(const uint8_t *data, size_t size, uint32_t *out_value,
                               size_t *out_consumed) {
    if (data == NULL || out_value == NULL || out_consumed == NULL) return MT_ERR_ARGUMENT;
    uint32_t value = 0u;
    unsigned shift = 0u;
    for (size_t i = 0u; i < size && i < 5u; ++i) {
        uint8_t byte = data[i];
        if (i == 4u && (byte & 0xf0u) != 0u) return MT_ERR_FORMAT;
        value |= (uint32_t)(byte & 0x7fu) << shift;
        if ((byte & 0x80u) == 0u) {
            *out_value = value;
            *out_consumed = i + 1u;
            return MT_OK;
        }
        shift += 7u;
    }
    return size < 5u ? MT_ERR_TRUNCATED : MT_ERR_FORMAT;
}

mt_status mt_varint_decode_i32(const uint8_t *data, size_t size, int32_t *out_value,
                               size_t *out_consumed) {
    uint32_t encoded;
    mt_status status = mt_varint_decode_u32(data, size, &encoded, out_consumed);
    if (status != MT_OK) return status;
    uint32_t magnitude = encoded >> 1u;
    int32_t value = (int32_t)magnitude;
    if ((encoded & 1u) != 0u) value = -value - 1;
    *out_value = value;
    return MT_OK;
}

size_t mt_varint_encode_u32(uint32_t value, uint8_t output[5]) {
    if (output == NULL) return 0u;
    size_t count = 0u;
    do {
        uint8_t byte = (uint8_t)(value & 0x7fu);
        value >>= 7u;
        if (value != 0u) byte |= 0x80u;
        output[count++] = byte;
    } while (value != 0u);
    return count;
}

size_t mt_varint_encode_i32(int32_t value, uint8_t output[5]) {
    uint32_t encoded = ((uint32_t)value << 1u) ^ (uint32_t)(value >> 31);
    return mt_varint_encode_u32(encoded, output);
}
