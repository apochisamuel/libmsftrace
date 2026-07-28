#include "semantic.h"

#include <stdlib.h>
#include <string.h>

mt_status mt_interval_index_init(mt_interval_index *index) {
    if (index == NULL) return MT_ERR_ARGUMENT;
    index->items = NULL;
    index->count = 0u;
    index->capacity = 0u;
    index->sorted = true;
    return MT_OK;
}

void mt_interval_index_dispose(mt_interval_index *index) {
    if (index == NULL) return;
    free(index->items);
    index->items = NULL;
    index->count = 0u;
    index->capacity = 0u;
    index->sorted = true;
}

static mt_status mt_interval_index_reserve(mt_interval_index *index, size_t capacity) {
    if (capacity <= index->capacity) return MT_OK;
    size_t next = index->capacity == 0u ? 16u : index->capacity;
    while (next < capacity) {
        if (next > SIZE_MAX / 2u) {
            next = capacity;
            break;
        }
        next *= 2u;
    }
    if (next > SIZE_MAX / sizeof(mt_interval)) return MT_ERR_NOMEM;
    mt_interval *items = (mt_interval *)realloc(index->items, next * sizeof(mt_interval));
    if (items == NULL) return MT_ERR_NOMEM;
    index->items = items;
    index->capacity = next;
    return MT_OK;
}

mt_status mt_interval_index_add(mt_interval_index *index, mt_interval interval) {
    if (index == NULL || interval.end < interval.start) return MT_ERR_ARGUMENT;
    mt_status status = mt_interval_index_reserve(index, index->count + 1u);
    if (status != MT_OK) return status;
    if (index->count != 0u && interval.start < index->items[index->count - 1u].start) {
        index->sorted = false;
    }
    index->items[index->count++] = interval;
    return MT_OK;
}

static int mt_compare_interval(const void *left, const void *right) {
    const mt_interval *a = (const mt_interval *)left;
    const mt_interval *b = (const mt_interval *)right;
    if (a->start != b->start) return a->start < b->start ? -1 : 1;
    if (a->end != b->end) return a->end > b->end ? -1 : 1;
    if (a->priority != b->priority) return a->priority > b->priority ? -1 : 1;
    if (a->payload != b->payload) return a->payload < b->payload ? -1 : 1;
    return 0;
}

mt_status mt_interval_index_finalize(mt_interval_index *index) {
    if (index == NULL) return MT_ERR_ARGUMENT;
    if (!index->sorted && index->count > 1u) {
        qsort(index->items, index->count, sizeof(mt_interval), mt_compare_interval);
    } else if (index->count > 1u) {
        qsort(index->items, index->count, sizeof(mt_interval), mt_compare_interval);
    }
    index->sorted = true;
    return MT_OK;
}

static bool mt_interval_contains(const mt_interval *interval, uint32_t point) {
    return interval->start <= point && point < interval->end;
}

bool mt_interval_index_find(const mt_interval_index *index, uint32_t point, mt_interval *out_interval) {
    if (index == NULL || !index->sorted || index->count == 0u) return false;
    size_t low = 0u;
    size_t high = index->count;
    while (low < high) {
        size_t mid = low + (high - low) / 2u;
        if (index->items[mid].start <= point) low = mid + 1u;
        else high = mid;
    }
    mt_interval best;
    bool found = false;
    size_t scan = low;
    while (scan > 0u) {
        --scan;
        const mt_interval *candidate = &index->items[scan];
        if (candidate->end <= point && found) break;
        if (!mt_interval_contains(candidate, point)) continue;
        if (!found || candidate->priority > best.priority ||
            (candidate->priority == best.priority && candidate->start > best.start) ||
            (candidate->priority == best.priority && candidate->start == best.start &&
             candidate->end < best.end)) {
            best = *candidate;
            found = true;
        }
    }
    if (found && out_interval != NULL) *out_interval = best;
    return found;
}

size_t mt_interval_index_collect(const mt_interval_index *index, uint32_t point,
                                 mt_interval *output, size_t capacity) {
    if (index == NULL || !index->sorted) return 0u;
    size_t count = 0u;
    for (size_t i = 0u; i < index->count; ++i) {
        if (index->items[i].start > point) break;
        if (!mt_interval_contains(&index->items[i], point)) continue;
        if (output != NULL && count < capacity) output[count] = index->items[i];
        ++count;
    }
    return count;
}
