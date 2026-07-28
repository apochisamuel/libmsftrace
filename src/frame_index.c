#include "semantic.h"

static bool mt_frame_contains(const mt_frame_interval *frame, uint16_t section, uint32_t source) {
    if (frame->section != section || frame->source_length == 0u || source < frame->source_start) {
        return false;
    }
    return source - frame->source_start < frame->source_length;
}

mt_status mt_lookup_frame_interval(const mt_frame_interval *frames, size_t frame_count,
                                   const mt_omap_segment *omap, size_t omap_count,
                                   uint16_t section, uint32_t optimized_offset,
                                   mt_frame_interval *out) {
    if (frames == NULL || omap == NULL || out == NULL) return MT_ERR_ARGUMENT;
    uint32_t source;
    mt_status status = mt_map_omap_reverse(omap, omap_count, optimized_offset, &source);
    if (status != MT_OK) return status;
    uint32_t transformed;
    status = mt_map_omap_reverse(omap, omap_count, source, &transformed);
    if (status == MT_OK) source = transformed;
    const mt_frame_interval *best = NULL;
    for (size_t i = 0u; i < frame_count; ++i) {
        if (!mt_frame_contains(&frames[i], section, source)) continue;
        if (best == NULL || frames[i].source_length < best->source_length ||
            (frames[i].source_length == best->source_length &&
             frames[i].source_start > best->source_start)) {
            best = &frames[i];
        }
    }
    if (best == NULL) return MT_ERR_NOT_FOUND;
    *out = *best;
    return MT_OK;
}
