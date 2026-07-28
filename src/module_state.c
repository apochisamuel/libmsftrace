#include "semantic.h"

static mt_module_state *mt_find_module(mt_module_state *modules, size_t module_count,
                                       uint32_t module_id) {
    for (size_t i = 0u; i < module_count; ++i) {
        if (modules[i].module_id == module_id) return &modules[i];
    }
    return NULL;
}

mt_status mt_apply_module_validation(mt_module_state *modules, size_t module_count,
                                     uint32_t module_id, bool symbols_valid,
                                     bool lines_valid, bool contributions_valid) {
    if (modules == NULL) return MT_ERR_ARGUMENT;
    mt_module_state *module = mt_find_module(modules, module_count, module_id);
    if (module == NULL) return MT_ERR_NOT_FOUND;
    module->symbols_valid = symbols_valid;
    module->lines_valid = lines_valid;
    module->contributions_valid = contributions_valid;
    module->included_in_symbol_queries = symbols_valid && lines_valid;
    module->included_in_address_queries = symbols_valid && contributions_valid;
    return MT_OK;
}
