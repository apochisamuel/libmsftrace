#ifndef MSFTRACE_SEMANTIC_H
#define MSFTRACE_SEMANTIC_H

#include "msftrace.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MT_SEM_NAME_CAP 96u
#define MT_SEM_MAX_DEPTH 128u
#define MT_SEM_INVALID UINT32_MAX

typedef enum mt_visibility {
    MT_VIS_LOCAL = 0,
    MT_VIS_MODULE = 1,
    MT_VIS_PUBLIC = 2
} mt_visibility;

typedef enum mt_symbol_class {
    MT_SYMBOL_DATA = 0,
    MT_SYMBOL_THUNK = 1,
    MT_SYMBOL_CODE = 2
} mt_symbol_class;

typedef struct mt_type_candidate {
    uint32_t domain;
    uint32_t index;
    uint16_t leaf;
    bool complete;
    bool forward;
    bool imported;
    const char *name;
} mt_type_candidate;

typedef struct mt_type_resolution {
    uint32_t canonical_domain;
    uint32_t canonical_index;
    uint32_t public_domain;
    uint32_t public_index;
    uint16_t leaf;
    const char *name;
} mt_type_resolution;

typedef struct mt_field_member {
    uint32_t id;
    uint32_t type_index;
    uint32_t declaration_order;
    uint32_t continuation;
    const char *name;
} mt_field_member;

typedef struct mt_field_record {
    uint32_t record_index;
    const mt_field_member *members;
    size_t member_count;
} mt_field_record;

typedef struct mt_identifier_key {
    uint32_t parent;
    uint32_t namespace_id;
    const char *text;
} mt_identifier_key;

typedef struct mt_identifier_entry {
    mt_identifier_key key;
    uint32_t canonical_id;
} mt_identifier_entry;

typedef struct mt_module_state {
    uint32_t module_id;
    uint32_t stream_index;
    bool symbols_valid;
    bool lines_valid;
    bool contributions_valid;
    bool included_in_symbol_queries;
    bool included_in_address_queries;
} mt_module_state;

typedef struct mt_contribution {
    uint32_t module_id;
    uint16_t section;
    uint32_t raw_start;
    uint32_t raw_length;
    uint32_t mapped_start;
    uint32_t mapped_length;
    uint32_t priority;
} mt_contribution;

typedef struct mt_public_record {
    uint32_t record_id;
    uint32_t module_id;
    uint16_t section;
    uint32_t offset;
    mt_visibility visibility;
    mt_symbol_class symbol_class;
    const char *name;
} mt_public_record;

typedef struct mt_gsi_entry {
    uint32_t hash;
    uint32_t canonical_id;
    uint32_t record_id;
    uint32_t next;
} mt_gsi_entry;

typedef struct mt_omap_segment {
    uint32_t source_start;
    uint32_t source_end;
    uint32_t target_start;
    bool gap;
} mt_omap_segment;

typedef enum mt_checksum_kind {
    MT_CHECKSUM_NONE = 0,
    MT_CHECKSUM_MD5 = 1,
    MT_CHECKSUM_SHA1 = 2,
    MT_CHECKSUM_SHA256 = 3
} mt_checksum_kind;

typedef struct mt_file_identity {
    const char *path;
    mt_checksum_kind kind;
    const uint8_t *digest;
    size_t digest_size;
} mt_file_identity;

typedef struct mt_inline_node {
    uint32_t node_id;
    uint32_t parent_id;
    uint32_t function_id;
    uint32_t file_id;
    uint32_t line;
    bool function_resolved;
} mt_inline_node;

typedef struct mt_inline_frame {
    uint32_t function_id;
    uint32_t file_id;
    uint32_t line;
} mt_inline_frame;

typedef struct mt_frame_interval {
    uint16_t section;
    uint32_t source_start;
    uint32_t source_length;
    uint32_t program_index;
} mt_frame_interval;

typedef struct mt_named_slot {
    uint32_t slot_id;
    uint32_t stream_index;
    uint32_t insertion_order;
    bool occupied;
    bool deleted;
    const char *name;
} mt_named_slot;

typedef struct mt_named_entry_view {
    uint32_t stream_index;
    const char *name;
} mt_named_entry_view;

typedef struct mt_u32_vector {
    uint32_t *items;
    size_t count;
    size_t capacity;
} mt_u32_vector;

typedef struct mt_byte_vector {
    uint8_t *items;
    size_t count;
    size_t capacity;
} mt_byte_vector;

typedef struct mt_interval {
    uint32_t start;
    uint32_t end;
    uint32_t payload;
    uint32_t priority;
} mt_interval;

typedef struct mt_interval_index {
    mt_interval *items;
    size_t count;
    size_t capacity;
    bool sorted;
} mt_interval_index;

typedef struct mt_string_builder {
    char *data;
    size_t length;
    size_t capacity;
} mt_string_builder;

mt_status mt_u32_vector_init(mt_u32_vector *vector);
void mt_u32_vector_dispose(mt_u32_vector *vector);
mt_status mt_u32_vector_push(mt_u32_vector *vector, uint32_t value);
mt_status mt_u32_vector_insert(mt_u32_vector *vector, size_t position, uint32_t value);
bool mt_u32_vector_remove(mt_u32_vector *vector, size_t position, uint32_t *out_value);
void mt_u32_vector_sort_unique(mt_u32_vector *vector);

mt_status mt_byte_vector_init(mt_byte_vector *vector);
void mt_byte_vector_dispose(mt_byte_vector *vector);
mt_status mt_byte_vector_append(mt_byte_vector *vector, const uint8_t *data, size_t size);
mt_status mt_byte_vector_append_u16(mt_byte_vector *vector, uint16_t value);
mt_status mt_byte_vector_append_u32(mt_byte_vector *vector, uint32_t value);

uint32_t mt_hash_bytes(const uint8_t *data, size_t size, uint32_t seed);
uint32_t mt_hash_string_ci(const char *text, uint32_t seed);
bool mt_string_equal_ci(const char *left, const char *right);
size_t mt_normalize_path(const char *path, char *output, size_t capacity);

mt_status mt_interval_index_init(mt_interval_index *index);
void mt_interval_index_dispose(mt_interval_index *index);
mt_status mt_interval_index_add(mt_interval_index *index, mt_interval interval);
mt_status mt_interval_index_finalize(mt_interval_index *index);
bool mt_interval_index_find(const mt_interval_index *index, uint32_t point, mt_interval *out_interval);
size_t mt_interval_index_collect(const mt_interval_index *index, uint32_t point,
                                 mt_interval *output, size_t capacity);

mt_status mt_string_builder_init(mt_string_builder *builder);
void mt_string_builder_dispose(mt_string_builder *builder);
mt_status mt_string_builder_append(mt_string_builder *builder, const char *text);
mt_status mt_string_builder_append_n(mt_string_builder *builder, const char *text, size_t length);
mt_status mt_string_builder_append_u32(mt_string_builder *builder, uint32_t value);
mt_status mt_string_builder_append_hex(mt_string_builder *builder, uint32_t value, unsigned width);
const char *mt_string_builder_data(const mt_string_builder *builder);

mt_status mt_select_type_definition(const mt_type_candidate *candidates, size_t count,
                                    uint32_t local_domain, mt_type_resolution *out);
mt_status mt_flatten_field_records(const mt_field_record *records, size_t record_count,
                                   uint32_t root_index, uint32_t *output,
                                   size_t output_capacity, size_t *out_count);
mt_status mt_build_identifier_table(const mt_identifier_key *keys, size_t count,
                                    mt_identifier_entry *output, size_t output_capacity,
                                    size_t *out_count);
mt_status mt_apply_module_validation(mt_module_state *modules, size_t module_count,
                                     uint32_t module_id, bool symbols_valid,
                                     bool lines_valid, bool contributions_valid);
mt_status mt_choose_contribution(const mt_contribution *items, size_t count,
                                 uint16_t section, uint32_t mapped_offset,
                                 mt_contribution *out);
mt_status mt_choose_public_symbol(const mt_public_record *records, size_t count,
                                  const char *name, uint16_t section, uint32_t offset,
                                  mt_public_record *out);
mt_status mt_rebuild_gsi(const mt_public_record *records, size_t record_count,
                         mt_gsi_entry *entries, size_t entry_capacity, size_t *out_count);
mt_status mt_gsi_lookup(const mt_gsi_entry *entries, size_t entry_count,
                        uint32_t hash, uint32_t *output, size_t capacity, size_t *out_count);
mt_status mt_normalize_omap(const mt_omap_segment *input, size_t input_count,
                            mt_omap_segment *output, size_t output_capacity,
                            size_t *out_count);
mt_status mt_map_omap_forward(const mt_omap_segment *segments, size_t count,
                              uint32_t source, uint32_t *out_target);
mt_status mt_map_omap_reverse(const mt_omap_segment *segments, size_t count,
                              uint32_t target, uint32_t *out_source);
bool mt_file_identity_equal(const mt_file_identity *left, const mt_file_identity *right);
mt_status mt_consolidate_file_identities(const mt_file_identity *input, size_t input_count,
                                         uint32_t *canonical_ids, size_t canonical_capacity,
                                         size_t *out_count);
mt_status mt_render_inline_chain(const mt_inline_node *nodes, size_t node_count,
                                 uint32_t leaf_id, mt_inline_frame *frames,
                                 size_t frame_capacity, size_t *out_count);
mt_status mt_lookup_frame_interval(const mt_frame_interval *frames, size_t frame_count,
                                   const mt_omap_segment *omap, size_t omap_count,
                                   uint16_t section, uint32_t optimized_offset,
                                   mt_frame_interval *out);
mt_status mt_compact_named_streams(const mt_named_slot *slots, size_t slot_count,
                                   mt_named_entry_view *output, size_t output_capacity,
                                   size_t *out_count);

const char *mt_cv_leaf_name(uint16_t leaf);
const char *mt_cv_symbol_name(uint16_t kind);
const char *mt_cv_machine_name(uint16_t machine);
const char *mt_cv_register_name(uint16_t machine, uint16_t reg);

#endif
