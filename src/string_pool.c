#include "semantic.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct mt_string_pool_entry { uint32_t hash; uint32_t offset; uint32_t length; uint32_t next; } mt_string_pool_entry;
typedef struct mt_string_pool { char *bytes; size_t byte_count; size_t byte_capacity; mt_string_pool_entry *entries; size_t entry_count; size_t entry_capacity; uint32_t *buckets; size_t bucket_count; } mt_string_pool;

static mt_status mt_pool_reserve_bytes(mt_string_pool *pool,size_t capacity){if(capacity<=pool->byte_capacity)return MT_OK;size_t next=pool->byte_capacity==0u?256u:pool->byte_capacity;while(next<capacity){if(next>SIZE_MAX/2u){next=capacity;break;}next*=2u;}char *bytes=(char*)realloc(pool->bytes,next);if(bytes==NULL)return MT_ERR_NOMEM;pool->bytes=bytes;pool->byte_capacity=next;return MT_OK;}
static mt_status mt_pool_reserve_entries(mt_string_pool *pool,size_t capacity){if(capacity<=pool->entry_capacity)return MT_OK;size_t next=pool->entry_capacity==0u?32u:pool->entry_capacity;while(next<capacity){if(next>SIZE_MAX/2u){next=capacity;break;}next*=2u;}if(next>SIZE_MAX/sizeof(mt_string_pool_entry))return MT_ERR_NOMEM;mt_string_pool_entry *entries=(mt_string_pool_entry*)realloc(pool->entries,next*sizeof(*entries));if(entries==NULL)return MT_ERR_NOMEM;pool->entries=entries;pool->entry_capacity=next;return MT_OK;}

mt_status mt_string_pool_init(mt_string_pool *pool,size_t bucket_count){if(pool==NULL||bucket_count==0u)return MT_ERR_ARGUMENT;memset(pool,0,sizeof(*pool));pool->buckets=(uint32_t*)malloc(bucket_count*sizeof(uint32_t));if(pool->buckets==NULL)return MT_ERR_NOMEM;for(size_t i=0u;i<bucket_count;++i)pool->buckets[i]=UINT32_MAX;pool->bucket_count=bucket_count;return MT_OK;}
void mt_string_pool_dispose(mt_string_pool *pool){if(pool==NULL)return;free(pool->bytes);free(pool->entries);free(pool->buckets);memset(pool,0,sizeof(*pool));}

mt_status mt_string_pool_intern(mt_string_pool *pool,const char *text,uint32_t *out_id){if(pool==NULL||text==NULL||out_id==NULL)return MT_ERR_ARGUMENT;size_t length=strlen(text);uint32_t hash=mt_hash_bytes((const uint8_t*)text,length,0x91e10da5u);size_t bucket=hash%pool->bucket_count;uint32_t current=pool->buckets[bucket];while(current!=UINT32_MAX){mt_string_pool_entry *entry=&pool->entries[current];if(entry->hash==hash&&entry->length==length&&memcmp(pool->bytes+entry->offset,text,length)==0){*out_id=current;return MT_OK;}current=entry->next;}
    if(length>UINT32_MAX||pool->byte_count>UINT32_MAX-length-1u||pool->entry_count>UINT32_MAX)return MT_ERR_NOMEM;mt_status s=mt_pool_reserve_bytes(pool,pool->byte_count+length+1u);if(s!=MT_OK)return s;s=mt_pool_reserve_entries(pool,pool->entry_count+1u);if(s!=MT_OK)return s;uint32_t id=(uint32_t)pool->entry_count;uint32_t offset=(uint32_t)pool->byte_count;memcpy(pool->bytes+pool->byte_count,text,length+1u);pool->byte_count+=length+1u;pool->entries[pool->entry_count++]=(mt_string_pool_entry){hash,offset,(uint32_t)length,pool->buckets[bucket]};pool->buckets[bucket]=id;*out_id=id;return MT_OK;}

const char *mt_string_pool_get(const mt_string_pool *pool,uint32_t id){if(pool==NULL||id>=pool->entry_count)return NULL;return pool->bytes+pool->entries[id].offset;}
size_t mt_string_pool_count(const mt_string_pool *pool){return pool==NULL?0u:pool->entry_count;}

mt_status mt_string_pool_rehash(mt_string_pool *pool,size_t bucket_count){if(pool==NULL||bucket_count==0u)return MT_ERR_ARGUMENT;uint32_t *buckets=(uint32_t*)malloc(bucket_count*sizeof(uint32_t));if(buckets==NULL)return MT_ERR_NOMEM;for(size_t i=0u;i<bucket_count;++i)buckets[i]=UINT32_MAX;for(size_t i=0u;i<pool->entry_count;++i){size_t bucket=pool->entries[i].hash%bucket_count;pool->entries[i].next=buckets[bucket];buckets[bucket]=(uint32_t)i;}free(pool->buckets);pool->buckets=buckets;pool->bucket_count=bucket_count;return MT_OK;}
