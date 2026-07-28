#include "semantic.h"

#include <string.h>

static bool mt_identifier_text_equal(const mt_identifier_key *left,
                                     const mt_identifier_key *right) {
    if (left->text == NULL || right->text == NULL) return left->text == right->text;
    return strcmp(left->text, right->text) == 0;
}

static bool mt_identifier_equivalent(const mt_identifier_key *left,
                                     const mt_identifier_key *right) {
    return left->namespace_id == right->namespace_id && mt_identifier_text_equal(left, right);
}

mt_status mt_build_identifier_table(const mt_identifier_key *keys, size_t count,
                                    mt_identifier_entry *output, size_t output_capacity,
                                    size_t *out_count) {
    if (keys == NULL || output == NULL || out_count == NULL) return MT_ERR_ARGUMENT;
    if (count > output_capacity) return MT_ERR_NOMEM;
    size_t unique = 0u;
    for (size_t i = 0u; i < count; ++i) {
        uint32_t canonical = (uint32_t)i;
        for (size_t j = 0u; j < i; ++j) {
            if (mt_identifier_equivalent(&keys[i], &keys[j])) {
                canonical = output[j].canonical_id;
                break;
            }
        }
        output[i].key = keys[i];
        output[i].canonical_id = canonical;
        if (canonical == i) ++unique;
    }
    *out_count = unique;
    return MT_OK;
}
