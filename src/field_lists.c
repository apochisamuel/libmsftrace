#include "semantic.h"

#include <stdlib.h>

static const mt_field_record *mt_find_field_record(const mt_field_record *records,
                                                    size_t record_count,
                                                    uint32_t record_index) {
    for (size_t i = 0u; i < record_count; ++i) {
        if (records[i].record_index == record_index) return &records[i];
    }
    return NULL;
}

static bool mt_seen_record(const uint32_t *stack, size_t depth, uint32_t index) {
    for (size_t i = 0u; i < depth; ++i) {
        if (stack[i] == index) return true;
    }
    return false;
}

static mt_status mt_flatten_record(const mt_field_record *records, size_t record_count,
                                   uint32_t record_index, uint32_t *stack, size_t depth,
                                   uint32_t *output, size_t output_capacity,
                                   size_t *written) {
    if (depth >= MT_SEM_MAX_DEPTH) return MT_ERR_FORMAT;
    if (mt_seen_record(stack, depth, record_index)) return MT_ERR_FORMAT;
    const mt_field_record *record = mt_find_field_record(records, record_count, record_index);
    if (record == NULL) return MT_ERR_NOT_FOUND;
    stack[depth] = record_index;
    for (size_t i = 0u; i < record->member_count; ++i) {
        const mt_field_member *member = &record->members[i];
        if (member->continuation != MT_SEM_INVALID) {
            mt_status status = mt_flatten_record(records, record_count, member->continuation,
                                                 stack, depth + 1u, output,
                                                 output_capacity, written);
            if (status != MT_OK) return status;
        }
        if (*written >= output_capacity) return MT_ERR_NOMEM;
        output[(*written)++] = member->id;
    }
    return MT_OK;
}

mt_status mt_flatten_field_records(const mt_field_record *records, size_t record_count,
                                   uint32_t root_index, uint32_t *output,
                                   size_t output_capacity, size_t *out_count) {
    if (records == NULL || output == NULL || out_count == NULL) return MT_ERR_ARGUMENT;
    uint32_t stack[MT_SEM_MAX_DEPTH];
    size_t written = 0u;
    mt_status status = mt_flatten_record(records, record_count, root_index, stack, 0u,
                                         output, output_capacity, &written);
    if (status != MT_OK) return status;
    *out_count = written;
    return MT_OK;
}
