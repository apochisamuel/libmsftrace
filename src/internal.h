#ifndef MSFTRACE_INTERNAL_H
#define MSFTRACE_INTERNAL_H

#include "msftrace.h"
#include "semantic.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MT_MAX_STREAMS 65536u
#define MT_MAX_BLOCK_SIZE (1u << 20)
#define MT_MAX_RECORDS (1u << 20)
#define MT_INVALID_INDEX UINT32_MAX

typedef struct mt_reader {
    const uint8_t *data;
    size_t size;
    size_t pos;
} mt_reader;

typedef struct mt_stream {
    uint32_t size;
    uint32_t block_count;
    uint32_t *blocks;
    uint8_t *materialized;
} mt_stream;

typedef struct mt_name_entry {
    uint32_t stream_index;
    char *name;
} mt_name_entry;

typedef struct mt_type_record {
    uint32_t index;
    uint16_t kind;
    char *name;
    uint32_t canonical_index;
} mt_type_record;

typedef struct mt_module {
    uint32_t stream_index;
    uint16_t section;
    uint32_t offset;
    uint32_t size;
    char *name;
} mt_module;

typedef struct mt_symbol {
    uint32_t stream_index;
    uint32_t record_offset;
    uint16_t section;
    uint32_t offset;
    char *name;
} mt_symbol;

typedef struct mt_line {
    uint16_t section;
    uint32_t offset;
    uint32_t line;
    uint32_t file_index;
} mt_line;

typedef struct mt_file_entry {
    char *path;
    uint32_t checksum;
} mt_file_entry;

typedef struct mt_omap_entry {
    uint32_t source;
    uint32_t target;
} mt_omap_entry;

typedef struct mt_frame {
    uint16_t section;
    uint32_t start;
    uint32_t length;
    uint32_t program_index;
} mt_frame;

struct mt_context {
    mt_status status;
    char message[192];
};

struct mt_database {
    mt_context *ctx;
    const uint8_t *input;
    size_t input_size;
    uint32_t block_size;
    uint32_t block_count;
    uint32_t directory_bytes;
    uint32_t directory_block_map;
    mt_stream *streams;
    uint32_t stream_count;
    mt_name_entry *names;
    uint32_t name_count;
    mt_type_record *types;
    uint32_t type_count;
    mt_module *modules;
    uint32_t module_count;
    mt_symbol *symbols;
    uint32_t symbol_count;
    mt_file_entry *files;
    uint32_t file_count;
    mt_line *lines;
    uint32_t line_count;
    mt_omap_entry *omap;
    uint32_t omap_count;
    mt_frame *frames;
    uint32_t frame_count;
};

void mt_set_error(mt_context *ctx, mt_status status, const char *message);
void *mt_calloc_array(mt_context *ctx, size_t count, size_t item_size);
char *mt_dup_bytes(mt_context *ctx, const uint8_t *data, size_t length);

void mt_reader_init(mt_reader *reader, const uint8_t *data, size_t size);
bool mt_read_u8(mt_reader *reader, uint8_t *out);
bool mt_read_u16(mt_reader *reader, uint16_t *out);
bool mt_read_u32(mt_reader *reader, uint32_t *out);
bool mt_read_bytes(mt_reader *reader, size_t length, const uint8_t **out);
bool mt_skip(mt_reader *reader, size_t length);

mt_status mt_parse_superblock(mt_database *db);
mt_status mt_parse_directory(mt_database *db);
mt_status mt_materialize_stream(mt_database *db, uint32_t index);
mt_status mt_parse_pdb_info(mt_database *db);
mt_status mt_parse_tpi(mt_database *db, uint32_t stream_index);
mt_status mt_parse_ipi(mt_database *db, uint32_t stream_index);
mt_status mt_parse_dbi(mt_database *db, uint32_t stream_index);
mt_status mt_parse_symbols(mt_database *db);
mt_status mt_parse_source_data(mt_database *db);
mt_status mt_parse_omap(mt_database *db);
mt_status mt_parse_frames(mt_database *db);
mt_status mt_validate_database(mt_database *db);
mt_status mt_build_indexes(mt_database *db);

#endif
