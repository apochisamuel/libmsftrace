#include "internal.h"

#include <stdlib.h>

mt_status mt_parse_frames(mt_database *db) {
    for (uint32_t i = 0; i < db->name_count; ++i) {
        if (db->names[i].name == NULL || db->names[i].name[0] != '/' ||
            db->names[i].name[1] != 'F') continue;
        uint32_t stream_index = db->names[i].stream_index;
        if (stream_index >= db->stream_count) continue;
        mt_status status = mt_materialize_stream(db, stream_index);
        if (status != MT_OK) return status;
        mt_stream *stream = &db->streams[stream_index];
        uint32_t count = stream->size / 14u;
        if (count > MT_MAX_RECORDS) return MT_ERR_FORMAT;
        db->frames = mt_calloc_array(db->ctx, count, sizeof(mt_frame));
        if (db->frames == NULL && count != 0u) return MT_ERR_NOMEM;
        mt_reader reader;
        mt_reader_init(&reader, stream->materialized, stream->size);
        while (db->frame_count < count) {
            mt_frame *frame = &db->frames[db->frame_count];
            if (!mt_read_u16(&reader, &frame->section) || !mt_read_u32(&reader, &frame->start) ||
                !mt_read_u32(&reader, &frame->length) || !mt_read_u32(&reader, &frame->program_index)) break;
            ++db->frame_count;
        }
        break;
    }
    return MT_OK;
}
