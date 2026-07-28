#include "internal.h"

#include <stdlib.h>

mt_status mt_parse_pdb_info(mt_database *db) {
    if (db->stream_count <= 1u) {
        return MT_OK;
    }
    mt_status status = mt_materialize_stream(db, 1u);
    if (status != MT_OK) {
        return status;
    }
    mt_stream *stream = &db->streams[1u];
    if (stream->size < 28u) {
        return MT_OK;
    }
    mt_reader reader;
    mt_reader_init(&reader, stream->materialized, stream->size);
    uint32_t version, signature, age;
    const uint8_t *guid;
    if (!mt_read_u32(&reader, &version) || !mt_read_u32(&reader, &signature) ||
        !mt_read_u32(&reader, &age) || !mt_read_bytes(&reader, 16u, &guid)) {
        return MT_ERR_TRUNCATED;
    }
    uint32_t names_size;
    if (!mt_read_u32(&reader, &names_size) || names_size > reader.size - reader.pos) {
        return MT_OK;
    }
    const uint8_t *names_data;
    if (!mt_read_bytes(&reader, names_size, &names_data)) {
        return MT_ERR_TRUNCATED;
    }
    uint32_t count = 0u;
    for (uint32_t i = 0; i < names_size; ++i) {
        if (names_data[i] == 0u) {
            ++count;
        }
    }
    db->names = mt_calloc_array(db->ctx, count, sizeof(mt_name_entry));
    if (db->names == NULL && count != 0u) {
        return MT_ERR_NOMEM;
    }
    uint32_t start = 0u;
    for (uint32_t i = 0; i < names_size && db->name_count < count; ++i) {
        if (names_data[i] != 0u) {
            continue;
        }
        uint32_t length = i - start;
        if (length != 0u) {
            db->names[db->name_count].name = mt_dup_bytes(db->ctx, names_data + start, length);
            if (db->names[db->name_count].name == NULL) {
                return MT_ERR_NOMEM;
            }
            db->names[db->name_count].stream_index = MT_INVALID_INDEX;
            ++db->name_count;
        }
        start = i + 1u;
    }
    (void)version;
    (void)signature;
    (void)age;
    (void)guid;
    return MT_OK;
}
