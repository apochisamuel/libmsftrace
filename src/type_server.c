#include "semantic.h"

#include <string.h>

static int mt_type_candidate_rank(const mt_type_candidate *candidate, uint32_t local_domain) {
    int rank = 0;
    if (candidate->complete) rank += 16;
    if (!candidate->forward) rank += 8;
    if (candidate->domain == local_domain) rank += 4;
    if (!candidate->imported) rank += 2;
    if (candidate->name != NULL && candidate->name[0] != '\0') rank += 1;
    return rank;
}

static bool mt_type_names_match(const mt_type_candidate *left, const mt_type_candidate *right) {
    if (left->name == NULL || right->name == NULL) return left->name == right->name;
    return strcmp(left->name, right->name) == 0;
}

mt_status mt_select_type_definition(const mt_type_candidate *candidates, size_t count,
                                    uint32_t local_domain, mt_type_resolution *out) {
    if (candidates == NULL || out == NULL || count == 0u) return MT_ERR_ARGUMENT;
    size_t best = 0u;
    int best_rank = mt_type_candidate_rank(&candidates[0], local_domain);
    for (size_t i = 1u; i < count; ++i) {
        if (!mt_type_names_match(&candidates[0], &candidates[i])) continue;
        int rank = mt_type_candidate_rank(&candidates[i], local_domain);
        if (rank > best_rank ||
            (rank == best_rank && candidates[i].index < candidates[best].index)) {
            best = i;
            best_rank = rank;
        }
    }
    out->canonical_domain = candidates[best].domain;
    out->canonical_index = candidates[best].index;
    out->public_domain = candidates[best].domain;
    out->public_index = candidates[best].index;
    out->leaf = candidates[best].leaf;
    out->name = candidates[best].name;
    return MT_OK;
}
