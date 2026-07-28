#include "internal.h"

#include <stdlib.h>

static mt_status mt_parse_type_stream(mt_database *db, uint32_t stream_index, uint32_t base_index) {
    if (stream_index >= db->stream_count) {
        return MT_OK;
    }
    mt_status status = mt_materialize_stream(db, stream_index);
    if (status != MT_OK) {
        return status;
    }
    mt_stream *stream = &db->streams[stream_index];
    if (stream->size < 16u) {
        return MT_OK;
    }
    mt_reader reader;
    mt_reader_init(&reader, stream->materialized, stream->size);
    uint32_t version, header_size, index_begin, index_end;
    if (!mt_read_u32(&reader, &version) || !mt_read_u32(&reader, &header_size) ||
        !mt_read_u32(&reader, &index_begin) || !mt_read_u32(&reader, &index_end)) {
        return MT_ERR_TRUNCATED;
    }
    if (header_size < 16u || header_size > stream->size || index_end < index_begin ||
        index_end - index_begin > MT_MAX_RECORDS) {
        return MT_OK;
    }
    reader.pos = header_size;
    uint32_t capacity = index_end - index_begin;
    mt_type_record *records = mt_calloc_array(db->ctx, capacity, sizeof(mt_type_record));
    if (records == NULL && capacity != 0u) {
        return MT_ERR_NOMEM;
    }
    uint32_t count = 0u;
    while (reader.pos + 4u <= reader.size && count < capacity) {
        uint16_t length, kind;
        if (!mt_read_u16(&reader, &length) || !mt_read_u16(&reader, &kind)) {
            break;
        }
        if (length < 2u || length - 2u > reader.size - reader.pos) {
            break;
        }
        const uint8_t *payload;
        uint16_t payload_length = (uint16_t)(length - 2u);
        if (!mt_read_bytes(&reader, payload_length, &payload)) {
            break;
        }
        size_t name_offset = 0u;
        while (name_offset < payload_length && payload[name_offset] < 0x20u) {
            ++name_offset;
        }
        size_t name_length = 0u;
        while (name_offset + name_length < payload_length &&
               payload[name_offset + name_length] != 0u) {
            ++name_length;
        }
        records[count].index = base_index + index_begin + count;
        records[count].kind = kind;
        records[count].canonical_index = records[count].index;
        records[count].name = mt_dup_bytes(db->ctx, payload + name_offset, name_length);
        if (records[count].name == NULL) {
            for (uint32_t i = 0; i < count; ++i) free(records[i].name);
            free(records);
            return MT_ERR_NOMEM;
        }
        ++count;
        reader.pos = (reader.pos + 3u) & ~(size_t)3u;
    }
    uint32_t old_count = db->type_count;
    mt_type_record *merged = mt_calloc_array(db->ctx, old_count + count, sizeof(mt_type_record));
    if (merged == NULL && old_count + count != 0u) {
        for (uint32_t i = 0; i < count; ++i) free(records[i].name);
        free(records);
        return MT_ERR_NOMEM;
    }
    for (uint32_t i = 0; i < old_count; ++i) merged[i] = db->types[i];
    for (uint32_t i = 0; i < count; ++i) merged[old_count + i] = records[i];
    free(db->types);
    free(records);
    db->types = merged;
    db->type_count = old_count + count;
    (void)version;
    return MT_OK;
}

mt_status mt_parse_tpi(mt_database *db, uint32_t stream_index) {
    return mt_parse_type_stream(db, stream_index, 0u);
}

mt_status mt_parse_ipi(mt_database *db, uint32_t stream_index) {
    return mt_parse_type_stream(db, stream_index, 0x10000000u);
}
