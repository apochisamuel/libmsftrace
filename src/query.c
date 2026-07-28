#include "internal.h"

#include <string.h>

mt_status mt_database_summary(const mt_database *db, mt_summary *out_summary) {
    if (db == NULL || out_summary == NULL) return MT_ERR_ARGUMENT;
    out_summary->block_size = db->block_size;
    out_summary->block_count = db->block_count;
    out_summary->stream_count = db->stream_count;
    out_summary->module_count = db->module_count;
    out_summary->type_count = db->type_count;
    out_summary->symbol_count = db->symbol_count;
    out_summary->source_file_count = db->file_count;
    return MT_OK;
}

mt_status mt_database_find_symbol(const mt_database *db, const char *name, mt_symbol_view *out_symbol) {
    if (db == NULL || name == NULL || out_symbol == NULL) return MT_ERR_ARGUMENT;
    for (uint32_t i = 0; i < db->symbol_count; ++i) {
        if (db->symbols[i].name != NULL && strcmp(db->symbols[i].name, name) == 0) {
            out_symbol->stream_index = db->symbols[i].stream_index;
            out_symbol->record_offset = db->symbols[i].record_offset;
            out_symbol->section = db->symbols[i].section;
            out_symbol->offset = db->symbols[i].offset;
            out_symbol->name = db->symbols[i].name;
            return MT_OK;
        }
    }
    return MT_ERR_NOT_FOUND;
}

mt_status mt_database_lookup_source(const mt_database *db,
                                    uint16_t section,
                                    uint32_t offset,
                                    mt_source_view *out_source) {
    if (db == NULL || out_source == NULL) return MT_ERR_ARGUMENT;
    const mt_line *best = NULL;
    for (uint32_t i = 0; i < db->line_count; ++i) {
        const mt_line *line = &db->lines[i];
        if (line->section != section || line->offset > offset) continue;
        if (best == NULL || line->offset > best->offset) best = line;
    }
    if (best == NULL || best->file_index == MT_INVALID_INDEX || best->file_index >= db->file_count) {
        return MT_ERR_NOT_FOUND;
    }
    out_source->section = section;
    out_source->offset = best->offset;
    out_source->line = best->line;
    out_source->path = db->files[best->file_index].path;
    return MT_OK;
}

mt_status mt_database_type_name(const mt_database *db, uint32_t type_index, const char **out_name) {
    if (db == NULL || out_name == NULL) return MT_ERR_ARGUMENT;
    for (uint32_t i = 0; i < db->type_count; ++i) {
        if (db->types[i].index == type_index) {
            *out_name = db->types[i].name;
            return MT_OK;
        }
    }
    return MT_ERR_NOT_FOUND;
}
