#include "semantic.h"

#include <stdlib.h>
#include <string.h>

static bool mt_digest_equal(const mt_file_identity *left, const mt_file_identity *right) {
    if (left->digest_size != right->digest_size) return false;
    if (left->digest_size == 0u) return true;
    if (left->digest == NULL || right->digest == NULL) return false;
    return memcmp(left->digest, right->digest, left->digest_size) == 0;
}

bool mt_file_identity_equal(const mt_file_identity *left, const mt_file_identity *right) {
    if (left == NULL || right == NULL) return false;
    char left_path[512];
    char right_path[512];
    size_t left_length = mt_normalize_path(left->path, left_path, sizeof(left_path));
    size_t right_length = mt_normalize_path(right->path, right_path, sizeof(right_path));
    if (left_length != right_length || strcmp(left_path, right_path) != 0) return false;
    return mt_digest_equal(left, right);
}

mt_status mt_consolidate_file_identities(const mt_file_identity *input, size_t input_count,
                                         uint32_t *canonical_ids, size_t canonical_capacity,
                                         size_t *out_count) {
    if (input == NULL || canonical_ids == NULL || out_count == NULL) return MT_ERR_ARGUMENT;
    if (input_count > canonical_capacity) return MT_ERR_NOMEM;
    size_t unique = 0u;
    for (size_t i = 0u; i < input_count; ++i) {
        canonical_ids[i] = (uint32_t)i;
        for (size_t j = 0u; j < i; ++j) {
            if (mt_file_identity_equal(&input[i], &input[j])) {
                canonical_ids[i] = canonical_ids[j];
                break;
            }
        }
        if (canonical_ids[i] == i) ++unique;
    }
    *out_count = unique;
    return MT_OK;
}
