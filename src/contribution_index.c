#include "semantic.h"

static bool mt_range_contains(uint32_t start, uint32_t length, uint32_t point) {
    if (length == 0u || point < start) return false;
    return point - start < length;
}

static bool mt_contribution_better(const mt_contribution *candidate,
                                   const mt_contribution *best,
                                   uint32_t mapped_offset) {
    bool candidate_raw = mt_range_contains(candidate->raw_start, candidate->raw_length, mapped_offset);
    bool best_raw = mt_range_contains(best->raw_start, best->raw_length, mapped_offset);
    if (candidate_raw != best_raw) return candidate_raw;
    if (candidate->priority != best->priority) return candidate->priority > best->priority;
    if (candidate->raw_length != best->raw_length) return candidate->raw_length < best->raw_length;
    return candidate->module_id < best->module_id;
}

mt_status mt_choose_contribution(const mt_contribution *items, size_t count,
                                 uint16_t section, uint32_t mapped_offset,
                                 mt_contribution *out) {
    if (items == NULL || out == NULL) return MT_ERR_ARGUMENT;
    const mt_contribution *best = NULL;
    for (size_t i = 0u; i < count; ++i) {
        const mt_contribution *candidate = &items[i];
        if (candidate->section != section) continue;
        if (!mt_range_contains(candidate->mapped_start, candidate->mapped_length, mapped_offset)) continue;
        if (best == NULL || mt_contribution_better(candidate, best, mapped_offset)) best = candidate;
    }
    if (best == NULL) return MT_ERR_NOT_FOUND;
    *out = *best;
    return MT_OK;
}
