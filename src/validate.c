#include "internal.h"

#include <stdlib.h>
#include <string.h>

static int mt_compare_types(const void *lhs, const void *rhs) {
    const mt_type_record *a = (const mt_type_record *)lhs;
    const mt_type_record *b = (const mt_type_record *)rhs;
    if (a->kind != b->kind) return a->kind < b->kind ? -1 : 1;
    const char *an = a->name == NULL ? "" : a->name;
    const char *bn = b->name == NULL ? "" : b->name;
    return strcmp(an, bn);
}

static int mt_compare_symbols(const void *lhs, const void *rhs) {
    const mt_symbol *a = (const mt_symbol *)lhs;
    const mt_symbol *b = (const mt_symbol *)rhs;
    if (a->section != b->section) return a->section < b->section ? -1 : 1;
    if (a->offset != b->offset) return a->offset < b->offset ? -1 : 1;
    return strcmp(a->name == NULL ? "" : a->name, b->name == NULL ? "" : b->name);
}

static int mt_compare_lines(const void *lhs, const void *rhs) {
    const mt_line *a = (const mt_line *)lhs;
    const mt_line *b = (const mt_line *)rhs;
    if (a->section != b->section) return a->section < b->section ? -1 : 1;
    if (a->offset != b->offset) return a->offset < b->offset ? -1 : 1;
    return a->line < b->line ? -1 : a->line != b->line;
}

mt_status mt_validate_database(mt_database *db) {
    for (uint32_t i = 0; i < db->module_count; ++i) {
        if (db->modules[i].stream_index >= db->stream_count) {
            db->modules[i].stream_index = MT_INVALID_INDEX;
        }
    }
    for (uint32_t i = 0; i < db->line_count; ++i) {
        if (db->lines[i].file_index >= db->file_count) {
            db->lines[i].file_index = MT_INVALID_INDEX;
        }
    }
    for (uint32_t i = 1; i < db->omap_count; ++i) {
        if (db->omap[i].source < db->omap[i - 1u].source) {
            mt_set_error(db->ctx, MT_ERR_FORMAT, "OMAP records are not ordered");
            return MT_ERR_FORMAT;
        }
    }
    return MT_OK;
}

mt_status mt_build_indexes(mt_database *db) {
    if (db->type_count > 1u) qsort(db->types, db->type_count, sizeof(mt_type_record), mt_compare_types);
    if (db->symbol_count > 1u) qsort(db->symbols, db->symbol_count, sizeof(mt_symbol), mt_compare_symbols);
    if (db->line_count > 1u) qsort(db->lines, db->line_count, sizeof(mt_line), mt_compare_lines);
    for (uint32_t i = 0; i < db->type_count; ++i) {
        db->types[i].canonical_index = db->types[i].index;
        if (i > 0u && db->types[i].kind == db->types[i - 1u].kind &&
            strcmp(db->types[i].name, db->types[i - 1u].name) == 0) {
            db->types[i].canonical_index = db->types[i - 1u].canonical_index;
        }
    }
    return MT_OK;
}
