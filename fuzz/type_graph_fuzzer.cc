#include "msftrace.h"
#include <cstddef>
#include <cstdint>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size < 64 || size > 16 * 1024 * 1024) return 0;
    mt_context *ctx = mt_context_create();
    mt_database *db = nullptr;
    if (ctx != nullptr && mt_database_open_memory(ctx, data, size, &db) == MT_OK) {
        mt_summary summary{};
        if (mt_database_summary(db, &summary) == MT_OK) {
            for (uint32_t i = 0; i < summary.type_count && i < 32u; ++i) {
                const char *name = nullptr;
                (void)mt_database_type_name(db, 0x1000u + i, &name);
            }
        }
        mt_database_close(db);
    }
    mt_context_destroy(ctx);
    return 0;
}
