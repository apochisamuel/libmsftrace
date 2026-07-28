#include "internal.h"

#include <string.h>

void mt_reader_init(mt_reader *reader, const uint8_t *data, size_t size) {
    reader->data = data;
    reader->size = size;
    reader->pos = 0;
}

static bool mt_reader_take(mt_reader *reader, size_t length, const uint8_t **out) {
    if (reader == NULL || length > reader->size - reader->pos) {
        return false;
    }
    if (out != NULL) {
        *out = reader->data + reader->pos;
    }
    reader->pos += length;
    return true;
}

bool mt_read_u8(mt_reader *reader, uint8_t *out) {
    const uint8_t *bytes;
    if (!mt_reader_take(reader, 1, &bytes)) {
        return false;
    }
    *out = bytes[0];
    return true;
}

bool mt_read_u16(mt_reader *reader, uint16_t *out) {
    const uint8_t *bytes;
    if (!mt_reader_take(reader, 2, &bytes)) {
        return false;
    }
    *out = (uint16_t)((uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8));
    return true;
}

bool mt_read_u32(mt_reader *reader, uint32_t *out) {
    const uint8_t *bytes;
    if (!mt_reader_take(reader, 4, &bytes)) {
        return false;
    }
    *out = (uint32_t)bytes[0] |
           ((uint32_t)bytes[1] << 8) |
           ((uint32_t)bytes[2] << 16) |
           ((uint32_t)bytes[3] << 24);
    return true;
}

bool mt_read_bytes(mt_reader *reader, size_t length, const uint8_t **out) {
    return mt_reader_take(reader, length, out);
}

bool mt_skip(mt_reader *reader, size_t length) {
    return mt_reader_take(reader, length, NULL);
}
