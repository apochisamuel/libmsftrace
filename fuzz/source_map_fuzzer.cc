#include "msftrace.h"
#include <cstddef>
#include <cstdint>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size < 64 || size > 16 * 1024 * 1024) return 0;
    mt_context *ctx = mt_context_create();
    mt_database *db = nullptr;
    if (ctx != nullptr && mt_database_open_memory(ctx, data, size, &db) == MT_OK) {
        for (uint16_t section = 1; section <= 4; ++section) {
            mt_source_view source{};
            (void)mt_database_lookup_source(db, section, static_cast<uint32_t>(size), &source);
        }
        mt_database_close(db);
    }
    mt_context_destroy(ctx);
    return 0;
}
