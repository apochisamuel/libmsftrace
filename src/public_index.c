#include "semantic.h"

#include <string.h>

static int mt_public_rank(const mt_public_record *record) {
    int rank = 0;
    if (record->visibility == MT_VIS_PUBLIC) rank += 16;
    else if (record->visibility == MT_VIS_MODULE) rank += 8;
    if (record->symbol_class == MT_SYMBOL_DATA) rank += 4;
    else if (record->symbol_class == MT_SYMBOL_CODE) rank += 2;
    if (record->module_id != MT_SEM_INVALID) rank += 1;
    return rank;
}

mt_status mt_choose_public_symbol(const mt_public_record *records, size_t count,
                                  const char *name, uint16_t section, uint32_t offset,
                                  mt_public_record *out) {
    if (records == NULL || name == NULL || out == NULL) return MT_ERR_ARGUMENT;
    const mt_public_record *best = NULL;
    int best_rank = -1;
    for (size_t i = 0u; i < count; ++i) {
        const mt_public_record *candidate = &records[i];
        if (candidate->name == NULL || strcmp(candidate->name, name) != 0) continue;
        if (candidate->section != section || candidate->offset != offset) continue;
        int rank = mt_public_rank(candidate);
        if (best == NULL || rank > best_rank ||
            (rank == best_rank && candidate->record_id < best->record_id)) {
            best = candidate;
            best_rank = rank;
        }
    }
    if (best == NULL) return MT_ERR_NOT_FOUND;
    *out = *best;
    return MT_OK;
}
