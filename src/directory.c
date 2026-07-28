#include "internal.h"

#include <stdlib.h>
#include <string.h>

static uint32_t mt_div_round_up(uint32_t value, uint32_t divisor) {
    return value == 0u ? 0u : 1u + (value - 1u) / divisor;
}

static mt_status mt_copy_blocks(mt_database *db,
                                const uint32_t *blocks,
                                uint32_t block_count,
                                uint8_t *output,
                                uint32_t output_size) {
    uint32_t copied = 0u;
    for (uint32_t i = 0; i < block_count; ++i) {
        if (blocks[i] >= db->block_count) {
            mt_set_error(db->ctx, MT_ERR_FORMAT, "stream references an invalid block");
            return MT_ERR_FORMAT;
        }
        uint32_t remaining = output_size - copied;
        uint32_t amount = remaining < db->block_size ? remaining : db->block_size;
        memcpy(output + copied, db->input + (size_t)blocks[i] * db->block_size, amount);
        copied += amount;
    }
    return MT_OK;
}

mt_status mt_parse_directory(mt_database *db) {
    uint32_t directory_blocks = mt_div_round_up(db->directory_bytes, db->block_size);
    if (directory_blocks == 0u || directory_blocks > db->block_size / 4u) {
        mt_set_error(db->ctx, MT_ERR_FORMAT, "directory block list is not representable");
        return MT_ERR_FORMAT;
    }
    const uint8_t *map_data = db->input + (size_t)db->directory_block_map * db->block_size;
    mt_reader map_reader;
    mt_reader_init(&map_reader, map_data, db->block_size);
    uint32_t *directory_block_ids = mt_calloc_array(db->ctx, directory_blocks, sizeof(uint32_t));
    if (directory_block_ids == NULL) {
        return MT_ERR_NOMEM;
    }
    for (uint32_t i = 0; i < directory_blocks; ++i) {
        if (!mt_read_u32(&map_reader, &directory_block_ids[i])) {
            free(directory_block_ids);
            mt_set_error(db->ctx, MT_ERR_TRUNCATED, "directory block map is incomplete");
            return MT_ERR_TRUNCATED;
        }
    }
    uint8_t *directory = mt_calloc_array(db->ctx, db->directory_bytes, 1u);
    if (directory == NULL) {
        free(directory_block_ids);
        return MT_ERR_NOMEM;
    }
    mt_status status = mt_copy_blocks(db, directory_block_ids, directory_blocks,
                                      directory, db->directory_bytes);
    free(directory_block_ids);
    if (status != MT_OK) {
        free(directory);
        return status;
    }

    mt_reader reader;
    mt_reader_init(&reader, directory, db->directory_bytes);
    if (!mt_read_u32(&reader, &db->stream_count) || db->stream_count > MT_MAX_STREAMS) {
        free(directory);
        mt_set_error(db->ctx, MT_ERR_FORMAT, "invalid stream count");
        return MT_ERR_FORMAT;
    }
    db->streams = mt_calloc_array(db->ctx, db->stream_count, sizeof(mt_stream));
    if (db->streams == NULL && db->stream_count != 0u) {
        free(directory);
        return MT_ERR_NOMEM;
    }
    for (uint32_t i = 0; i < db->stream_count; ++i) {
        uint32_t size;
        if (!mt_read_u32(&reader, &size)) {
            free(directory);
            mt_set_error(db->ctx, MT_ERR_TRUNCATED, "stream size table is incomplete");
            return MT_ERR_TRUNCATED;
        }
        db->streams[i].size = size == UINT32_MAX ? 0u : size;
        db->streams[i].block_count = mt_div_round_up(db->streams[i].size, db->block_size);
    }
    for (uint32_t i = 0; i < db->stream_count; ++i) {
        mt_stream *stream = &db->streams[i];
        stream->blocks = mt_calloc_array(db->ctx, stream->block_count, sizeof(uint32_t));
        if (stream->blocks == NULL && stream->block_count != 0u) {
            free(directory);
            return MT_ERR_NOMEM;
        }
        for (uint32_t j = 0; j < stream->block_count; ++j) {
            if (!mt_read_u32(&reader, &stream->blocks[j])) {
                free(directory);
                mt_set_error(db->ctx, MT_ERR_TRUNCATED, "stream block table is incomplete");
                return MT_ERR_TRUNCATED;
            }
        }
    }
    free(directory);
    return MT_OK;
}

mt_status mt_materialize_stream(mt_database *db, uint32_t index) {
    if (index >= db->stream_count) {
        return MT_ERR_NOT_FOUND;
    }
    mt_stream *stream = &db->streams[index];
    if (stream->materialized != NULL || stream->size == 0u) {
        return MT_OK;
    }
    stream->materialized = mt_calloc_array(db->ctx, stream->size, 1u);
    if (stream->materialized == NULL) {
        return MT_ERR_NOMEM;
    }
    mt_status status = mt_copy_blocks(db, stream->blocks, stream->block_count,
                                      stream->materialized, stream->size);
    if (status != MT_OK) {
        free(stream->materialized);
        stream->materialized = NULL;
    }
    return status;
}
