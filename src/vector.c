#include "semantic.h"

#include <stdlib.h>
#include <string.h>

static bool mt_size_multiply(size_t left, size_t right, size_t *out) {
    if (out == NULL) return false;
    if (left != 0u && right > SIZE_MAX / left) return false;
    *out = left * right;
    return true;
}

static mt_status mt_u32_vector_reserve(mt_u32_vector *vector, size_t capacity) {
    if (vector == NULL) return MT_ERR_ARGUMENT;
    if (capacity <= vector->capacity) return MT_OK;
    size_t next = vector->capacity == 0u ? 8u : vector->capacity;
    while (next < capacity) {
        if (next > SIZE_MAX / 2u) {
            next = capacity;
            break;
        }
        next *= 2u;
    }
    size_t bytes;
    if (!mt_size_multiply(next, sizeof(uint32_t), &bytes)) return MT_ERR_NOMEM;
    uint32_t *items = (uint32_t *)realloc(vector->items, bytes);
    if (items == NULL) return MT_ERR_NOMEM;
    vector->items = items;
    vector->capacity = next;
    return MT_OK;
}

mt_status mt_u32_vector_init(mt_u32_vector *vector) {
    if (vector == NULL) return MT_ERR_ARGUMENT;
    vector->items = NULL;
    vector->count = 0u;
    vector->capacity = 0u;
    return MT_OK;
}

void mt_u32_vector_dispose(mt_u32_vector *vector) {
    if (vector == NULL) return;
    free(vector->items);
    vector->items = NULL;
    vector->count = 0u;
    vector->capacity = 0u;
}

mt_status mt_u32_vector_push(mt_u32_vector *vector, uint32_t value) {
    if (vector == NULL) return MT_ERR_ARGUMENT;
    mt_status status = mt_u32_vector_reserve(vector, vector->count + 1u);
    if (status != MT_OK) return status;
    vector->items[vector->count++] = value;
    return MT_OK;
}

mt_status mt_u32_vector_insert(mt_u32_vector *vector, size_t position, uint32_t value) {
    if (vector == NULL || position > vector->count) return MT_ERR_ARGUMENT;
    mt_status status = mt_u32_vector_reserve(vector, vector->count + 1u);
    if (status != MT_OK) return status;
    if (position < vector->count) {
        memmove(&vector->items[position + 1u], &vector->items[position],
                (vector->count - position) * sizeof(uint32_t));
    }
    vector->items[position] = value;
    ++vector->count;
    return MT_OK;
}

bool mt_u32_vector_remove(mt_u32_vector *vector, size_t position, uint32_t *out_value) {
    if (vector == NULL || position >= vector->count) return false;
    if (out_value != NULL) *out_value = vector->items[position];
    if (position + 1u < vector->count) {
        memmove(&vector->items[position], &vector->items[position + 1u],
                (vector->count - position - 1u) * sizeof(uint32_t));
    }
    --vector->count;
    return true;
}

static int mt_compare_u32(const void *left, const void *right) {
    uint32_t a = *(const uint32_t *)left;
    uint32_t b = *(const uint32_t *)right;
    if (a < b) return -1;
    if (a > b) return 1;
    return 0;
}

void mt_u32_vector_sort_unique(mt_u32_vector *vector) {
    if (vector == NULL || vector->count < 2u) return;
    qsort(vector->items, vector->count, sizeof(uint32_t), mt_compare_u32);
    size_t write = 1u;
    for (size_t read = 1u; read < vector->count; ++read) {
        if (vector->items[read] != vector->items[write - 1u]) {
            vector->items[write++] = vector->items[read];
        }
    }
    vector->count = write;
}

static mt_status mt_byte_vector_reserve(mt_byte_vector *vector, size_t capacity) {
    if (vector == NULL) return MT_ERR_ARGUMENT;
    if (capacity <= vector->capacity) return MT_OK;
    size_t next = vector->capacity == 0u ? 64u : vector->capacity;
    while (next < capacity) {
        if (next > SIZE_MAX / 2u) {
            next = capacity;
            break;
        }
        next *= 2u;
    }
    uint8_t *items = (uint8_t *)realloc(vector->items, next);
    if (items == NULL) return MT_ERR_NOMEM;
    vector->items = items;
    vector->capacity = next;
    return MT_OK;
}

mt_status mt_byte_vector_init(mt_byte_vector *vector) {
    if (vector == NULL) return MT_ERR_ARGUMENT;
    vector->items = NULL;
    vector->count = 0u;
    vector->capacity = 0u;
    return MT_OK;
}

void mt_byte_vector_dispose(mt_byte_vector *vector) {
    if (vector == NULL) return;
    free(vector->items);
    vector->items = NULL;
    vector->count = 0u;
    vector->capacity = 0u;
}

mt_status mt_byte_vector_append(mt_byte_vector *vector, const uint8_t *data, size_t size) {
    if (vector == NULL || (data == NULL && size != 0u)) return MT_ERR_ARGUMENT;
    if (size > SIZE_MAX - vector->count) return MT_ERR_NOMEM;
    mt_status status = mt_byte_vector_reserve(vector, vector->count + size);
    if (status != MT_OK) return status;
    if (size != 0u) memcpy(vector->items + vector->count, data, size);
    vector->count += size;
    return MT_OK;
}

mt_status mt_byte_vector_append_u16(mt_byte_vector *vector, uint16_t value) {
    uint8_t data[2];
    data[0] = (uint8_t)(value & 0xffu);
    data[1] = (uint8_t)((value >> 8u) & 0xffu);
    return mt_byte_vector_append(vector, data, sizeof(data));
}

mt_status mt_byte_vector_append_u32(mt_byte_vector *vector, uint32_t value) {
    uint8_t data[4];
    data[0] = (uint8_t)(value & 0xffu);
    data[1] = (uint8_t)((value >> 8u) & 0xffu);
    data[2] = (uint8_t)((value >> 16u) & 0xffu);
    data[3] = (uint8_t)((value >> 24u) & 0xffu);
    return mt_byte_vector_append(vector, data, sizeof(data));
}
