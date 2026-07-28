#include "semantic.h"

#include <stddef.h>
#include <stdint.h>

static const char mt_base64_alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

size_t mt_base64_encoded_size(size_t input_size) {
    if (input_size > (SIZE_MAX - 2u) / 3u) return SIZE_MAX;
    return ((input_size + 2u) / 3u) * 4u;
}

mt_status mt_base64_encode(const uint8_t *input, size_t input_size,
                           char *output, size_t output_capacity,
                           size_t *out_size) {
    if ((input == NULL && input_size != 0u) || output == NULL || out_size == NULL) return MT_ERR_ARGUMENT;
    size_t needed = mt_base64_encoded_size(input_size);
    if (needed == SIZE_MAX || output_capacity < needed + 1u) return MT_ERR_NOMEM;
    size_t read = 0u;
    size_t write = 0u;
    while (read + 3u <= input_size) {
        uint32_t value = ((uint32_t)input[read] << 16u) |
                         ((uint32_t)input[read + 1u] << 8u) |
                         (uint32_t)input[read + 2u];
        output[write++] = mt_base64_alphabet[(value >> 18u) & 63u];
        output[write++] = mt_base64_alphabet[(value >> 12u) & 63u];
        output[write++] = mt_base64_alphabet[(value >> 6u) & 63u];
        output[write++] = mt_base64_alphabet[value & 63u];
        read += 3u;
    }
    size_t remaining = input_size - read;
    if (remaining == 1u) {
        uint32_t value = (uint32_t)input[read] << 16u;
        output[write++] = mt_base64_alphabet[(value >> 18u) & 63u];
        output[write++] = mt_base64_alphabet[(value >> 12u) & 63u];
        output[write++] = '=';
        output[write++] = '=';
    } else if (remaining == 2u) {
        uint32_t value = ((uint32_t)input[read] << 16u) | ((uint32_t)input[read + 1u] << 8u);
        output[write++] = mt_base64_alphabet[(value >> 18u) & 63u];
        output[write++] = mt_base64_alphabet[(value >> 12u) & 63u];
        output[write++] = mt_base64_alphabet[(value >> 6u) & 63u];
        output[write++] = '=';
    }
    output[write] = '\0';
    *out_size = write;
    return MT_OK;
}

static int mt_base64_value(unsigned char ch) {
    if (ch >= 'A' && ch <= 'Z') return ch - 'A';
    if (ch >= 'a' && ch <= 'z') return 26 + ch - 'a';
    if (ch >= '0' && ch <= '9') return 52 + ch - '0';
    if (ch == '+') return 62;
    if (ch == '/') return 63;
    return -1;
}

mt_status mt_base64_decode(const char *input, size_t input_size,
                           uint8_t *output, size_t output_capacity,
                           size_t *out_size) {
    if (input == NULL || output == NULL || out_size == NULL) return MT_ERR_ARGUMENT;
    if ((input_size % 4u) != 0u) return MT_ERR_FORMAT;
    size_t write = 0u;
    for (size_t read = 0u; read < input_size; read += 4u) {
        int a = mt_base64_value((unsigned char)input[read]);
        int b = mt_base64_value((unsigned char)input[read + 1u]);
        bool pad_c = input[read + 2u] == '=';
        bool pad_d = input[read + 3u] == '=';
        int c = pad_c ? 0 : mt_base64_value((unsigned char)input[read + 2u]);
        int d = pad_d ? 0 : mt_base64_value((unsigned char)input[read + 3u]);
        if (a < 0 || b < 0 || c < 0 || d < 0 || (pad_c && !pad_d) ||
            ((pad_c || pad_d) && read + 4u != input_size)) return MT_ERR_FORMAT;
        uint32_t value = ((uint32_t)a << 18u) | ((uint32_t)b << 12u) |
                         ((uint32_t)c << 6u) | (uint32_t)d;
        size_t bytes = pad_c ? 1u : pad_d ? 2u : 3u;
        if (bytes > output_capacity - write) return MT_ERR_NOMEM;
        output[write++] = (uint8_t)(value >> 16u);
        if (bytes > 1u) output[write++] = (uint8_t)(value >> 8u);
        if (bytes > 2u) output[write++] = (uint8_t)value;
    }
    *out_size = write;
    return MT_OK;
}
