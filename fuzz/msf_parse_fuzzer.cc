#include "msftrace.h"
#include <cstddef>
#include <cstdint>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size < 64 || size > 16 * 1024 * 1024) return 0;
    mt_context *ctx = mt_context_create();
    if (ctx == nullptr) return 0;
    mt_database *db = nullptr;
    if (mt_database_open_memory(ctx, data, size, &db) == MT_OK) {
        mt_summary summary{};
        (void)mt_database_summary(db, &summary);
        mt_database_close(db);
    }
    mt_context_destroy(ctx);
    return 0;
}
