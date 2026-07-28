#include "internal.h"

#include <stdlib.h>

mt_status mt_parse_dbi(mt_database *db, uint32_t stream_index) {
    if (stream_index >= db->stream_count) {
        return MT_OK;
    }
    mt_status status = mt_materialize_stream(db, stream_index);
    if (status != MT_OK) {
        return status;
    }
    mt_stream *stream = &db->streams[stream_index];
    if (stream->size < 32u) {
        return MT_OK;
    }
    mt_reader reader;
    mt_reader_init(&reader, stream->materialized, stream->size);
    uint32_t signature, version, age, module_bytes;
    uint16_t global_stream, public_stream;
    if (!mt_read_u32(&reader, &signature) || !mt_read_u32(&reader, &version) ||
        !mt_read_u32(&reader, &age) || !mt_read_u16(&reader, &global_stream) ||
        !mt_read_u16(&reader, &public_stream) || !mt_read_u32(&reader, &module_bytes)) {
        return MT_ERR_TRUNCATED;
    }
    if (module_bytes > reader.size - reader.pos) {
        module_bytes = (uint32_t)(reader.size - reader.pos);
    }
    size_t module_end = reader.pos + module_bytes;
    uint32_t capacity = module_bytes / 16u + 1u;
    db->modules = mt_calloc_array(db->ctx, capacity, sizeof(mt_module));
    if (db->modules == NULL && capacity != 0u) {
        return MT_ERR_NOMEM;
    }
    while (reader.pos + 16u <= module_end && db->module_count < capacity) {
        mt_module *module = &db->modules[db->module_count];
        uint16_t name_length;
        if (!mt_read_u32(&reader, &module->stream_index) ||
            !mt_read_u16(&reader, &module->section) ||
            !mt_read_u32(&reader, &module->offset) ||
            !mt_read_u32(&reader, &module->size) ||
            !mt_read_u16(&reader, &name_length)) {
            break;
        }
        if (name_length > module_end - reader.pos) {
            break;
        }
        const uint8_t *name;
        if (!mt_read_bytes(&reader, name_length, &name)) {
            break;
        }
        module->name = mt_dup_bytes(db->ctx, name, name_length);
        if (module->name == NULL) {
            return MT_ERR_NOMEM;
        }
        ++db->module_count;
        reader.pos = (reader.pos + 3u) & ~(size_t)3u;
    }
    (void)signature;
    (void)version;
    (void)age;
    (void)global_stream;
    (void)public_stream;
    return MT_OK;
}
