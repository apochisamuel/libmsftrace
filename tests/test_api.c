#include "msftrace.h"
#include <assert.h>
#include <stdint.h>

int main(void) {
    uint8_t data[64] = {0};
    mt_context *ctx = mt_context_create();
    assert(ctx != 0);
    mt_database *db = 0;
    assert(mt_database_open_memory(ctx, data, sizeof(data), &db) == MT_ERR_FORMAT);
    assert(db == 0);
    mt_context_destroy(ctx);
    return 0;
}
