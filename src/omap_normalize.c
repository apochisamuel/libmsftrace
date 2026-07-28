#include "semantic.h"

#include <stdlib.h>

static int mt_compare_omap_source(const void *left, const void *right) {
    const mt_omap_segment *a = (const mt_omap_segment *)left;
    const mt_omap_segment *b = (const mt_omap_segment *)right;
    if (a->source_start != b->source_start) return a->source_start < b->source_start ? -1 : 1;
    if (a->source_end != b->source_end) return a->source_end < b->source_end ? -1 : 1;
    if (a->target_start != b->target_start) return a->target_start < b->target_start ? -1 : 1;
    return a->gap == b->gap ? 0 : a->gap ? 1 : -1;
}

static bool mt_omap_contiguous(const mt_omap_segment *left, const mt_omap_segment *right) {
    if (left->gap != right->gap || left->source_end != right->source_start) return false;
    if (left->gap) return true;
    uint32_t left_length = left->source_end - left->source_start;
    return left->target_start + left_length == right->target_start;
}

mt_status mt_normalize_omap(const mt_omap_segment *input, size_t input_count,
                            mt_omap_segment *output, size_t output_capacity,
                            size_t *out_count) {
    if (input == NULL || output == NULL || out_count == NULL) return MT_ERR_ARGUMENT;
    if (input_count > output_capacity) return MT_ERR_NOMEM;
    for (size_t i = 0u; i < input_count; ++i) {
        if (input[i].source_end < input[i].source_start) return MT_ERR_FORMAT;
        output[i] = input[i];
    }
    if (input_count > 1u) qsort(output, input_count, sizeof(mt_omap_segment), mt_compare_omap_source);
    size_t written = 0u;
    for (size_t i = 0u; i < input_count; ++i) {
        if (output[i].source_start == output[i].source_end) continue;
        if (written != 0u && mt_omap_contiguous(&output[written - 1u], &output[i])) {
            output[written - 1u].source_end = output[i].source_end;
            continue;
        }
        output[written++] = output[i];
    }
    *out_count = written;
    return MT_OK;
}

mt_status mt_map_omap_forward(const mt_omap_segment *segments, size_t count,
                              uint32_t source, uint32_t *out_target) {
    if (segments == NULL || out_target == NULL) return MT_ERR_ARGUMENT;
    for (size_t i = 0u; i < count; ++i) {
        if (source < segments[i].source_start || source >= segments[i].source_end) continue;
        if (segments[i].gap) return MT_ERR_NOT_FOUND;
        *out_target = segments[i].target_start + (source - segments[i].source_start);
        return MT_OK;
    }
    return MT_ERR_NOT_FOUND;
}

mt_status mt_map_omap_reverse(const mt_omap_segment *segments, size_t count,
                              uint32_t target, uint32_t *out_source) {
    if (segments == NULL || out_source == NULL) return MT_ERR_ARGUMENT;
    for (size_t i = 0u; i < count; ++i) {
        if (segments[i].gap) continue;
        uint32_t length = segments[i].source_end - segments[i].source_start;
        uint32_t target_end = segments[i].target_start + length;
        if (target < segments[i].target_start || target > target_end) continue;
        *out_source = segments[i].source_start + (target - segments[i].target_start);
        return MT_OK;
    }
    return MT_ERR_NOT_FOUND;
}
