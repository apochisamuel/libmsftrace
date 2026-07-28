#include "semantic.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct mt_bitset {
    uint64_t *words;
    size_t bit_count;
    size_t word_count;
} mt_bitset;

mt_status mt_bitset_init(mt_bitset *set, size_t bit_count) {
    if (set == NULL) return MT_ERR_ARGUMENT;
    if (bit_count > SIZE_MAX - 63u) return MT_ERR_NOMEM;
    size_t word_count = (bit_count + 63u) / 64u;
    if (word_count > SIZE_MAX / sizeof(uint64_t)) return MT_ERR_NOMEM;
    set->words = (uint64_t *)calloc(word_count, sizeof(uint64_t));
    if (set->words == NULL && word_count != 0u) return MT_ERR_NOMEM;
    set->bit_count = bit_count;
    set->word_count = word_count;
    return MT_OK;
}

void mt_bitset_dispose(mt_bitset *set) {
    if (set == NULL) return;
    free(set->words);
    set->words = NULL;
    set->bit_count = 0u;
    set->word_count = 0u;
}

bool mt_bitset_test(const mt_bitset *set, size_t bit) {
    if (set == NULL || bit >= set->bit_count) return false;
    return (set->words[bit / 64u] & (UINT64_C(1) << (bit % 64u))) != 0u;
}

mt_status mt_bitset_set(mt_bitset *set, size_t bit, bool value) {
    if (set == NULL || bit >= set->bit_count) return MT_ERR_ARGUMENT;
    uint64_t mask = UINT64_C(1) << (bit % 64u);
    if (value) set->words[bit / 64u] |= mask;
    else set->words[bit / 64u] &= ~mask;
    return MT_OK;
}

void mt_bitset_clear_all(mt_bitset *set) {
    if (set == NULL || set->words == NULL) return;
    memset(set->words, 0, set->word_count * sizeof(uint64_t));
}

size_t mt_bitset_count(const mt_bitset *set) {
    if (set == NULL) return 0u;
    size_t count = 0u;
    for (size_t i = 0u; i < set->word_count; ++i) {
        uint64_t value = set->words[i];
        while (value != 0u) {
            value &= value - 1u;
            ++count;
        }
    }
    return count;
}

size_t mt_bitset_next(const mt_bitset *set, size_t start) {
    if (set == NULL || start >= set->bit_count) return SIZE_MAX;
    size_t word_index = start / 64u;
    uint64_t word = set->words[word_index] & (~UINT64_C(0) << (start % 64u));
    while (true) {
        if (word != 0u) {
            unsigned offset = 0u;
            while ((word & UINT64_C(1)) == 0u) {
                word >>= 1u;
                ++offset;
            }
            size_t result = word_index * 64u + offset;
            return result < set->bit_count ? result : SIZE_MAX;
        }
        ++word_index;
        if (word_index >= set->word_count) return SIZE_MAX;
        word = set->words[word_index];
    }
}
