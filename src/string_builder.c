#include "semantic.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static mt_status mt_string_builder_reserve(mt_string_builder *builder, size_t capacity) {
    if (builder == NULL) return MT_ERR_ARGUMENT;
    if (capacity <= builder->capacity) return MT_OK;
    size_t next = builder->capacity == 0u ? 64u : builder->capacity;
    while (next < capacity) {
        if (next > SIZE_MAX / 2u) {
            next = capacity;
            break;
        }
        next *= 2u;
    }
    char *data = (char *)realloc(builder->data, next);
    if (data == NULL) return MT_ERR_NOMEM;
    builder->data = data;
    builder->capacity = next;
    return MT_OK;
}

mt_status mt_string_builder_init(mt_string_builder *builder) {
    if (builder == NULL) return MT_ERR_ARGUMENT;
    builder->data = NULL;
    builder->length = 0u;
    builder->capacity = 0u;
    mt_status status = mt_string_builder_reserve(builder, 1u);
    if (status == MT_OK) builder->data[0] = '\0';
    return status;
}

void mt_string_builder_dispose(mt_string_builder *builder) {
    if (builder == NULL) return;
    free(builder->data);
    builder->data = NULL;
    builder->length = 0u;
    builder->capacity = 0u;
}

mt_status mt_string_builder_append_n(mt_string_builder *builder, const char *text, size_t length) {
    if (builder == NULL || (text == NULL && length != 0u)) return MT_ERR_ARGUMENT;
    if (length > SIZE_MAX - builder->length - 1u) return MT_ERR_NOMEM;
    mt_status status = mt_string_builder_reserve(builder, builder->length + length + 1u);
    if (status != MT_OK) return status;
    if (length != 0u) memcpy(builder->data + builder->length, text, length);
    builder->length += length;
    builder->data[builder->length] = '\0';
    return MT_OK;
}

mt_status mt_string_builder_append(mt_string_builder *builder, const char *text) {
    if (text == NULL) return MT_ERR_ARGUMENT;
    return mt_string_builder_append_n(builder, text, strlen(text));
}

mt_status mt_string_builder_append_u32(mt_string_builder *builder, uint32_t value) {
    char buffer[16];
    int length = snprintf(buffer, sizeof(buffer), "%u", value);
    if (length < 0 || (size_t)length >= sizeof(buffer)) return MT_ERR_FORMAT;
    return mt_string_builder_append_n(builder, buffer, (size_t)length);
}

mt_status mt_string_builder_append_hex(mt_string_builder *builder, uint32_t value, unsigned width) {
    if (width > 8u) width = 8u;
    char format[8];
    int format_length = snprintf(format, sizeof(format), "%%0%ux", width);
    if (format_length < 0 || (size_t)format_length >= sizeof(format)) return MT_ERR_FORMAT;
    char buffer[16];
    int length = snprintf(buffer, sizeof(buffer), format, value);
    if (length < 0 || (size_t)length >= sizeof(buffer)) return MT_ERR_FORMAT;
    return mt_string_builder_append_n(builder, buffer, (size_t)length);
}

const char *mt_string_builder_data(const mt_string_builder *builder) {
    if (builder == NULL || builder->data == NULL) return "";
    return builder->data;
}
