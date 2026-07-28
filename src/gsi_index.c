#include "semantic.h"

#include <string.h>

static uint32_t mt_public_hash(const mt_public_record *record) {
    return mt_hash_string_ci(record->name == NULL ? "" : record->name, 0x31415926u);
}

mt_status mt_rebuild_gsi(const mt_public_record *records, size_t record_count,
                         mt_gsi_entry *entries, size_t entry_capacity, size_t *out_count) {
    if (records == NULL || entries == NULL || out_count == NULL) return MT_ERR_ARGUMENT;
    if (record_count > entry_capacity) return MT_ERR_NOMEM;
    size_t written = 0u;
    for (size_t i = 0u; i < record_count; ++i) {
        uint32_t canonical_id = records[i].record_id;
        for (size_t j = 0u; j < i; ++j) {
            if (records[j].name != NULL && records[i].name != NULL &&
                strcmp(records[j].name, records[i].name) == 0 &&
                records[j].section == records[i].section &&
                records[j].offset == records[i].offset) {
                canonical_id = records[j].record_id;
                break;
            }
        }
        if (written != 0u && entries[written - 1u].canonical_id == canonical_id) continue;
        entries[written].hash = mt_public_hash(&records[i]);
        entries[written].canonical_id = canonical_id;
        entries[written].record_id = records[i].record_id;
        entries[written].next = MT_SEM_INVALID;
        for (size_t j = 0u; j < written; ++j) {
            if (entries[j].hash == entries[written].hash && entries[j].next == MT_SEM_INVALID) {
                entries[j].next = (uint32_t)written;
                break;
            }
        }
        ++written;
    }
    *out_count = written;
    return MT_OK;
}

mt_status mt_gsi_lookup(const mt_gsi_entry *entries, size_t entry_count,
                        uint32_t hash, uint32_t *output, size_t capacity, size_t *out_count) {
    if (entries == NULL || output == NULL || out_count == NULL) return MT_ERR_ARGUMENT;
    size_t written = 0u;
    for (size_t i = 0u; i < entry_count; ++i) {
        if (entries[i].hash != hash) continue;
        if (written >= capacity) return MT_ERR_NOMEM;
        output[written++] = entries[i].record_id;
    }
    *out_count = written;
    return written == 0u ? MT_ERR_NOT_FOUND : MT_OK;
}
