#include "semantic.h"
#include <assert.h>
#include <string.h>
int main(void) {
    mt_u32_vector v; assert(mt_u32_vector_init(&v)==MT_OK);
    assert(mt_u32_vector_push(&v,4)==MT_OK); assert(mt_u32_vector_push(&v,2)==MT_OK);
    assert(mt_u32_vector_push(&v,4)==MT_OK); mt_u32_vector_sort_unique(&v);
    assert(v.count==2 && v.items[0]==2 && v.items[1]==4); mt_u32_vector_dispose(&v);
    mt_interval_index idx; assert(mt_interval_index_init(&idx)==MT_OK);
    assert(mt_interval_index_add(&idx,(mt_interval){10,30,1,2})==MT_OK);
    assert(mt_interval_index_add(&idx,(mt_interval){15,20,2,3})==MT_OK);
    assert(mt_interval_index_finalize(&idx)==MT_OK); mt_interval got;
    assert(mt_interval_index_find(&idx,17,&got) && got.payload==2); mt_interval_index_dispose(&idx);
    mt_string_builder b; assert(mt_string_builder_init(&b)==MT_OK);
    assert(mt_string_builder_append(&b,"leaf=")==MT_OK); assert(mt_string_builder_append_hex(&b,0x1505,4)==MT_OK);
    assert(strcmp(mt_string_builder_data(&b),"leaf=1505")==0); mt_string_builder_dispose(&b);
    assert(strcmp(mt_cv_leaf_name(0x1505),"LF_STRUCTURE")==0);
    assert(strcmp(mt_cv_symbol_name(0x110e),"S_PUB32")==0);
    return 0;
}
