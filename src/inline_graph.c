#include "semantic.h"

static const mt_inline_node *mt_find_inline_node(const mt_inline_node *nodes,
                                                 size_t node_count,
                                                 uint32_t node_id) {
    for (size_t i = 0u; i < node_count; ++i) {
        if (nodes[i].node_id == node_id) return &nodes[i];
    }
    return NULL;
}

static bool mt_chain_contains(const uint32_t *visited, size_t count, uint32_t node_id) {
    for (size_t i = 0u; i < count; ++i) {
        if (visited[i] == node_id) return true;
    }
    return false;
}

mt_status mt_render_inline_chain(const mt_inline_node *nodes, size_t node_count,
                                 uint32_t leaf_id, mt_inline_frame *frames,
                                 size_t frame_capacity, size_t *out_count) {
    if (nodes == NULL || frames == NULL || out_count == NULL) return MT_ERR_ARGUMENT;
    uint32_t visited[MT_SEM_MAX_DEPTH];
    mt_inline_frame reverse[MT_SEM_MAX_DEPTH];
    size_t depth = 0u;
    uint32_t current = leaf_id;
    while (current != MT_SEM_INVALID) {
        if (depth >= MT_SEM_MAX_DEPTH || mt_chain_contains(visited, depth, current)) {
            return MT_ERR_FORMAT;
        }
        const mt_inline_node *node = mt_find_inline_node(nodes, node_count, current);
        if (node == NULL) return MT_ERR_NOT_FOUND;
        visited[depth] = current;
        if (!node->function_resolved) {
            depth = 0u;
            current = node->parent_id;
            continue;
        }
        reverse[depth].function_id = node->function_id;
        reverse[depth].file_id = node->file_id;
        reverse[depth].line = node->line;
        ++depth;
        current = node->parent_id;
    }
    if (depth > frame_capacity) return MT_ERR_NOMEM;
    for (size_t i = 0u; i < depth; ++i) frames[i] = reverse[depth - i - 1u];
    *out_count = depth;
    return depth == 0u ? MT_ERR_NOT_FOUND : MT_OK;
}
