#ifndef MSFTRACE_H
#define MSFTRACE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mt_context mt_context;
typedef struct mt_database mt_database;

typedef enum mt_status {
    MT_OK = 0,
    MT_ERR_ARGUMENT,
    MT_ERR_NOMEM,
    MT_ERR_TRUNCATED,
    MT_ERR_FORMAT,
    MT_ERR_UNSUPPORTED,
    MT_ERR_NOT_FOUND
} mt_status;

typedef struct mt_summary {
    uint32_t block_size;
    uint32_t block_count;
    uint32_t stream_count;
    uint32_t module_count;
    uint32_t type_count;
    uint32_t symbol_count;
    uint32_t source_file_count;
} mt_summary;

typedef struct mt_symbol_view {
    uint32_t stream_index;
    uint32_t record_offset;
    uint16_t section;
    uint32_t offset;
    const char *name;
} mt_symbol_view;

typedef struct mt_source_view {
    uint16_t section;
    uint32_t offset;
    uint32_t line;
    const char *path;
} mt_source_view;

mt_context *mt_context_create(void);
void mt_context_destroy(mt_context *ctx);
const char *mt_context_message(const mt_context *ctx);

mt_status mt_database_open_memory(mt_context *ctx,
                                  const uint8_t *data,
                                  size_t size,
                                  mt_database **out_db);
void mt_database_close(mt_database *db);
mt_status mt_database_summary(const mt_database *db, mt_summary *out_summary);
mt_status mt_database_find_symbol(const mt_database *db,
                                  const char *name,
                                  mt_symbol_view *out_symbol);
mt_status mt_database_lookup_source(const mt_database *db,
                                    uint16_t section,
                                    uint32_t offset,
                                    mt_source_view *out_source);
mt_status mt_database_type_name(const mt_database *db,
                                uint32_t type_index,
                                const char **out_name);

#ifdef __cplusplus
}
#endif

#endif
