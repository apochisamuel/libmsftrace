#include "internal.h"

#include <string.h>

static const uint8_t mt_msf7_magic[32] = {
    'M','i','c','r','o','s','o','f','t',' ','C','/','C','+','+',' ','M','S','F',' ','7','.','0','0','\r','\n',0x1a,'D','S',0,0,0
};

mt_status mt_parse_superblock(mt_database *db) {
    if (db == NULL || db->input_size < 56u) {
        return MT_ERR_TRUNCATED;
    }
    if (memcmp(db->input, mt_msf7_magic, sizeof(mt_msf7_magic)) != 0) {
        mt_set_error(db->ctx, MT_ERR_FORMAT, "unsupported MSF signature");
        return MT_ERR_FORMAT;
    }
    mt_reader reader;
    mt_reader_init(&reader, db->input + 32u, db->input_size - 32u);
    uint32_t free_map;
    uint32_t unknown;
    if (!mt_read_u32(&reader, &db->block_size) ||
        !mt_read_u32(&reader, &free_map) ||
        !mt_read_u32(&reader, &db->block_count) ||
        !mt_read_u32(&reader, &db->directory_bytes) ||
        !mt_read_u32(&reader, &unknown) ||
        !mt_read_u32(&reader, &db->directory_block_map)) {
        mt_set_error(db->ctx, MT_ERR_TRUNCATED, "incomplete MSF superblock");
        return MT_ERR_TRUNCATED;
    }
    if (db->block_size < 512u || db->block_size > MT_MAX_BLOCK_SIZE ||
        (db->block_size & (db->block_size - 1u)) != 0u) {
        mt_set_error(db->ctx, MT_ERR_FORMAT, "invalid MSF block size");
        return MT_ERR_FORMAT;
    }
    if (db->block_count == 0u || (uint64_t)db->block_count * db->block_size > db->input_size) {
        mt_set_error(db->ctx, MT_ERR_TRUNCATED, "MSF block geometry exceeds input");
        return MT_ERR_TRUNCATED;
    }
    if (db->directory_block_map >= db->block_count) {
        mt_set_error(db->ctx, MT_ERR_FORMAT, "directory map block is outside the file");
        return MT_ERR_FORMAT;
    }
    (void)free_map;
    (void)unknown;
    return MT_OK;
}
