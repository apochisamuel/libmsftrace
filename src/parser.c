#include "internal.h"

#include <stdlib.h>

static void mt_free_database(mt_database *db) {
    if (db == NULL) return;
    for (uint32_t i = 0; i < db->stream_count; ++i) {
        free(db->streams[i].blocks);
        free(db->streams[i].materialized);
    }
    for (uint32_t i = 0; i < db->name_count; ++i) free(db->names[i].name);
    for (uint32_t i = 0; i < db->type_count; ++i) free(db->types[i].name);
    for (uint32_t i = 0; i < db->module_count; ++i) free(db->modules[i].name);
    for (uint32_t i = 0; i < db->symbol_count; ++i) free(db->symbols[i].name);
    for (uint32_t i = 0; i < db->file_count; ++i) free(db->files[i].path);
    free(db->streams);
    free(db->names);
    free(db->types);
    free(db->modules);
    free(db->symbols);
    free(db->files);
    free(db->lines);
    free(db->omap);
    free(db->frames);
    free(db);
}

mt_status mt_database_open_memory(mt_context *ctx,
                                  const uint8_t *data,
                                  size_t size,
                                  mt_database **out_db) {
    if (ctx == NULL || data == NULL || out_db == NULL) return MT_ERR_ARGUMENT;
    *out_db = NULL;
    mt_database *db = mt_calloc_array(ctx, 1u, sizeof(mt_database));
    if (db == NULL) return MT_ERR_NOMEM;
    db->ctx = ctx;
    db->input = data;
    db->input_size = size;
    mt_status status = mt_parse_superblock(db);
    if (status == MT_OK) status = mt_parse_directory(db);
    if (status == MT_OK) status = mt_parse_pdb_info(db);
    if (status == MT_OK) status = mt_parse_tpi(db, 2u);
    if (status == MT_OK) status = mt_parse_dbi(db, 3u);
    if (status == MT_OK) status = mt_parse_ipi(db, 4u);
    if (status == MT_OK) status = mt_parse_symbols(db);
    if (status == MT_OK) status = mt_parse_source_data(db);
    if (status == MT_OK) status = mt_parse_omap(db);
    if (status == MT_OK) status = mt_parse_frames(db);
    if (status == MT_OK) status = mt_validate_database(db);
    if (status == MT_OK) status = mt_build_indexes(db);
    if (status != MT_OK) {
        mt_free_database(db);
        return status;
    }
    *out_db = db;
    return MT_OK;
}

void mt_database_close(mt_database *db) {
    mt_free_database(db);
}
