#include "internal.h"

#include <stdlib.h>

static mt_status mt_add_file(mt_database *db, const uint8_t *path, uint16_t length, uint32_t checksum) {
    mt_file_entry *next = mt_calloc_array(db->ctx, db->file_count + 1u, sizeof(mt_file_entry));
    if (next == NULL) return MT_ERR_NOMEM;
    for (uint32_t i = 0; i < db->file_count; ++i) next[i] = db->files[i];
    next[db->file_count].path = mt_dup_bytes(db->ctx, path, length);
    if (next[db->file_count].path == NULL) {
        free(next);
        return MT_ERR_NOMEM;
    }
    next[db->file_count].checksum = checksum;
    free(db->files);
    db->files = next;
    ++db->file_count;
    return MT_OK;
}

static mt_status mt_add_line(mt_database *db, const mt_line *line) {
    mt_line *next = mt_calloc_array(db->ctx, db->line_count + 1u, sizeof(mt_line));
    if (next == NULL) return MT_ERR_NOMEM;
    for (uint32_t i = 0; i < db->line_count; ++i) next[i] = db->lines[i];
    next[db->line_count] = *line;
    free(db->lines);
    db->lines = next;
    ++db->line_count;
    return MT_OK;
}

mt_status mt_parse_source_data(mt_database *db) {
    for (uint32_t m = 0; m < db->module_count; ++m) {
        uint32_t stream_index = db->modules[m].stream_index;
        if (stream_index >= db->stream_count) continue;
        mt_status status = mt_materialize_stream(db, stream_index);
        if (status != MT_OK) return status;
        mt_stream *stream = &db->streams[stream_index];
        if (stream->size < 4u) continue;
        mt_reader reader;
        mt_reader_init(&reader, stream->materialized, stream->size);
        uint32_t subsection_count;
        if (!mt_read_u32(&reader, &subsection_count) || subsection_count > 4096u) continue;
        for (uint32_t s = 0; s < subsection_count && reader.pos + 8u <= reader.size; ++s) {
            uint16_t kind, item_count;
            uint32_t byte_count;
            if (!mt_read_u16(&reader, &kind) || !mt_read_u16(&reader, &item_count) ||
                !mt_read_u32(&reader, &byte_count) || byte_count > reader.size - reader.pos) break;
            size_t end = reader.pos + byte_count;
            if (kind == 1u) {
                for (uint16_t i = 0; i < item_count && reader.pos + 6u <= end; ++i) {
                    uint16_t length;
                    uint32_t checksum;
                    if (!mt_read_u16(&reader, &length) || !mt_read_u32(&reader, &checksum) ||
                        length > end - reader.pos) break;
                    const uint8_t *path;
                    if (!mt_read_bytes(&reader, length, &path)) break;
                    status = mt_add_file(db, path, length, checksum);
                    if (status != MT_OK) return status;
                }
            } else if (kind == 2u) {
                for (uint16_t i = 0; i < item_count && reader.pos + 14u <= end; ++i) {
                    mt_line line;
                    if (!mt_read_u16(&reader, &line.section) || !mt_read_u32(&reader, &line.offset) ||
                        !mt_read_u32(&reader, &line.line) || !mt_read_u32(&reader, &line.file_index)) break;
                    status = mt_add_line(db, &line);
                    if (status != MT_OK) return status;
                }
            }
            reader.pos = end;
        }
    }
    return MT_OK;
}
