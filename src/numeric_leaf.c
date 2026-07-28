#include "semantic.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef struct mt_numeric_value {
    bool is_signed;
    bool is_float;
    uint64_t unsigned_value;
    int64_t signed_value;
    long double real_value;
} mt_numeric_value;

static uint16_t mt_load_u16(const uint8_t *p) {
    return (uint16_t)p[0] | (uint16_t)((uint16_t)p[1] << 8u);
}

static uint32_t mt_load_u32(const uint8_t *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8u) |
           ((uint32_t)p[2] << 16u) | ((uint32_t)p[3] << 24u);
}

static uint64_t mt_load_u64(const uint8_t *p) {
    uint64_t low = mt_load_u32(p);
    uint64_t high = mt_load_u32(p + 4u);
    return low | (high << 32u);
}

mt_status mt_numeric_leaf_decode(const uint8_t *data, size_t size, mt_numeric_value *out,
                                 size_t *out_consumed) {
    if (data == NULL || out == NULL || out_consumed == NULL) return MT_ERR_ARGUMENT;
    if (size < 2u) return MT_ERR_TRUNCATED;
    memset(out, 0, sizeof(*out));
    uint16_t leaf = mt_load_u16(data);
    if (leaf < 0x8000u) {
        out->unsigned_value = leaf;
        *out_consumed = 2u;
        return MT_OK;
    }
    switch (leaf) {
        case 0x8000u:
            if (size < 3u) return MT_ERR_TRUNCATED;
            out->is_signed = true;
            out->signed_value = (int8_t)data[2];
            *out_consumed = 3u;
            return MT_OK;
        case 0x8001u:
            if (size < 4u) return MT_ERR_TRUNCATED;
            out->is_signed = true;
            out->signed_value = (int16_t)mt_load_u16(data + 2u);
            *out_consumed = 4u;
            return MT_OK;
        case 0x8002u:
            if (size < 4u) return MT_ERR_TRUNCATED;
            out->unsigned_value = mt_load_u16(data + 2u);
            *out_consumed = 4u;
            return MT_OK;
        case 0x8003u:
            if (size < 6u) return MT_ERR_TRUNCATED;
            out->is_signed = true;
            out->signed_value = (int32_t)mt_load_u32(data + 2u);
            *out_consumed = 6u;
            return MT_OK;
        case 0x8004u:
            if (size < 6u) return MT_ERR_TRUNCATED;
            out->unsigned_value = mt_load_u32(data + 2u);
            *out_consumed = 6u;
            return MT_OK;
        case 0x8009u:
            if (size < 10u) return MT_ERR_TRUNCATED;
            out->is_signed = true;
            out->signed_value = (int64_t)mt_load_u64(data + 2u);
            *out_consumed = 10u;
            return MT_OK;
        case 0x800au:
            if (size < 10u) return MT_ERR_TRUNCATED;
            out->unsigned_value = mt_load_u64(data + 2u);
            *out_consumed = 10u;
            return MT_OK;
        case 0x8005u: {
            if (size < 6u) return MT_ERR_TRUNCATED;
            uint32_t raw = mt_load_u32(data + 2u);
            float value;
            memcpy(&value, &raw, sizeof(value));
            out->is_float = true;
            out->real_value = value;
            *out_consumed = 6u;
            return MT_OK;
        }
        case 0x8006u: {
            if (size < 10u) return MT_ERR_TRUNCATED;
            uint64_t raw = mt_load_u64(data + 2u);
            double value;
            memcpy(&value, &raw, sizeof(value));
            out->is_float = true;
            out->real_value = value;
            *out_consumed = 10u;
            return MT_OK;
        }
        default:
            return MT_ERR_UNSUPPORTED;
    }
}
