#include "internal.h"

#include <stdlib.h>

mt_status mt_parse_omap(mt_database *db) {
    for (uint32_t i = 0; i < db->name_count; ++i) {
        if (db->names[i].name == NULL) continue;
        const char *name = db->names[i].name;
        if (name[0] != '/' || name[1] != 'O' || name[2] != 'M') continue;
        uint32_t stream_index = db->names[i].stream_index;
        if (stream_index >= db->stream_count) continue;
        mt_status status = mt_materialize_stream(db, stream_index);
        if (status != MT_OK) return status;
        mt_stream *stream = &db->streams[stream_index];
        uint32_t count = stream->size / 8u;
        if (count > MT_MAX_RECORDS) return MT_ERR_FORMAT;
        db->omap = mt_calloc_array(db->ctx, count, sizeof(mt_omap_entry));
        if (db->omap == NULL && count != 0u) return MT_ERR_NOMEM;
        mt_reader reader;
        mt_reader_init(&reader, stream->materialized, stream->size);
        while (db->omap_count < count) {
            mt_omap_entry *entry = &db->omap[db->omap_count];
            if (!mt_read_u32(&reader, &entry->source) || !mt_read_u32(&reader, &entry->target)) break;
            ++db->omap_count;
        }
        break;
    }
    return MT_OK;
}
