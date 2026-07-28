#include "internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

mt_context *mt_context_create(void) {
    mt_context *ctx = (mt_context *)calloc(1, sizeof(*ctx));
    if (ctx != NULL) {
        ctx->status = MT_OK;
        snprintf(ctx->message, sizeof(ctx->message), "%s", "ok");
    }
    return ctx;
}

void mt_context_destroy(mt_context *ctx) {
    free(ctx);
}

const char *mt_context_message(const mt_context *ctx) {
    return ctx == NULL ? "invalid context" : ctx->message;
}

void mt_set_error(mt_context *ctx, mt_status status, const char *message) {
    if (ctx == NULL) {
        return;
    }
    ctx->status = status;
    snprintf(ctx->message, sizeof(ctx->message), "%s", message == NULL ? "error" : message);
}

void *mt_calloc_array(mt_context *ctx, size_t count, size_t item_size) {
    if (item_size != 0 && count > SIZE_MAX / item_size) {
        mt_set_error(ctx, MT_ERR_NOMEM, "allocation size exceeds addressable range");
        return NULL;
    }
    void *ptr = calloc(count, item_size);
    if (ptr == NULL && count != 0) {
        mt_set_error(ctx, MT_ERR_NOMEM, "allocation failed");
    }
    return ptr;
}

char *mt_dup_bytes(mt_context *ctx, const uint8_t *data, size_t length) {
    char *copy = (char *)mt_calloc_array(ctx, length + 1u, 1u);
    if (copy == NULL) {
        return NULL;
    }
    if (length != 0) {
        memcpy(copy, data, length);
    }
    copy[length] = '\0';
    return copy;
}
