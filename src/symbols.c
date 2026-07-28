#include "internal.h"

#include <stdlib.h>

static mt_status mt_append_symbol(mt_database *db, const mt_symbol *symbol) {
    if (db->symbol_count >= MT_MAX_RECORDS) {
        return MT_ERR_FORMAT;
    }
    mt_symbol *next = mt_calloc_array(db->ctx, db->symbol_count + 1u, sizeof(mt_symbol));
    if (next == NULL) {
        return MT_ERR_NOMEM;
    }
    for (uint32_t i = 0; i < db->symbol_count; ++i) next[i] = db->symbols[i];
    next[db->symbol_count] = *symbol;
    free(db->symbols);
    db->symbols = next;
    ++db->symbol_count;
    return MT_OK;
}

static mt_status mt_parse_symbol_stream(mt_database *db, uint32_t stream_index) {
    if (stream_index >= db->stream_count) return MT_OK;
    mt_status status = mt_materialize_stream(db, stream_index);
    if (status != MT_OK) return status;
    mt_stream *stream = &db->streams[stream_index];
    mt_reader reader;
    mt_reader_init(&reader, stream->materialized, stream->size);
    while (reader.pos + 12u <= reader.size) {
        uint16_t length, kind, section, name_length;
        uint32_t offset;
        size_t record_offset = reader.pos;
        if (!mt_read_u16(&reader, &length) || !mt_read_u16(&reader, &kind)) break;
        if (length < 10u || length - 2u > reader.size - reader.pos) break;
        size_t record_end = record_offset + 2u + length;
        if (!mt_read_u16(&reader, &section) || !mt_read_u32(&reader, &offset) ||
            !mt_read_u16(&reader, &name_length) || name_length > record_end - reader.pos) break;
        const uint8_t *name;
        if (!mt_read_bytes(&reader, name_length, &name)) break;
        mt_symbol symbol;
        symbol.stream_index = stream_index;
        symbol.record_offset = (uint32_t)record_offset;
        symbol.section = section;
        symbol.offset = offset;
        symbol.name = mt_dup_bytes(db->ctx, name, name_length);
        if (symbol.name == NULL) return MT_ERR_NOMEM;
        status = mt_append_symbol(db, &symbol);
        if (status != MT_OK) {
            free(symbol.name);
            return status;
        }
        reader.pos = (record_end + 3u) & ~(size_t)3u;
        (void)kind;
    }
    return MT_OK;
}

mt_status mt_parse_symbols(mt_database *db) {
    for (uint32_t i = 0; i < db->module_count; ++i) {
        mt_status status = mt_parse_symbol_stream(db, db->modules[i].stream_index);
        if (status != MT_OK) return status;
    }
    return MT_OK;
}
