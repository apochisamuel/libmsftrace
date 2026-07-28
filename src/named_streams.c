#include "semantic.h"

#include <stdlib.h>
#include <string.h>

static int mt_compare_named_view(const void *left, const void *right) {
    const mt_named_entry_view *a = (const mt_named_entry_view *)left;
    const mt_named_entry_view *b = (const mt_named_entry_view *)right;
    if (a->stream_index != b->stream_index) return a->stream_index < b->stream_index ? -1 : 1;
    if (a->name == NULL || b->name == NULL) return a->name == b->name ? 0 : a->name == NULL ? -1 : 1;
    return strcmp(a->name, b->name);
}

mt_status mt_compact_named_streams(const mt_named_slot *slots, size_t slot_count,
                                   mt_named_entry_view *output, size_t output_capacity,
                                   size_t *out_count) {
    if (slots == NULL || output == NULL || out_count == NULL) return MT_ERR_ARGUMENT;
    size_t written = 0u;
    for (size_t i = 0u; i < slot_count; ++i) {
        if (!slots[i].occupied || slots[i].deleted || slots[i].name == NULL) continue;
        bool replaced = false;
        for (size_t j = 0u; j < written; ++j) {
            if (output[j].stream_index != slots[i].stream_index) continue;
            output[j].name = slots[i].name;
            replaced = true;
            break;
        }
        if (replaced) continue;
        if (written >= output_capacity) return MT_ERR_NOMEM;
        output[written].stream_index = slots[i].stream_index;
        output[written].name = slots[i].name;
        ++written;
    }
    if (written > 1u) qsort(output, written, sizeof(mt_named_entry_view), mt_compare_named_view);
    *out_count = written;
    return MT_OK;
}
