#include "semantic.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum mt_json_scope_kind { MT_JSON_OBJECT, MT_JSON_ARRAY } mt_json_scope_kind;
typedef struct mt_json_scope { mt_json_scope_kind kind; bool first; bool expects_value; } mt_json_scope;
typedef struct mt_json_writer { mt_string_builder builder; mt_json_scope scopes[64]; size_t depth; } mt_json_writer;

static mt_status mt_json_raw(mt_json_writer *writer,const char *text){return mt_string_builder_append(&writer->builder,text);}
static mt_status mt_json_prefix(mt_json_writer *writer){if(writer->depth==0u)return MT_OK;mt_json_scope *scope=&writer->scopes[writer->depth-1u];if(scope->kind==MT_JSON_OBJECT){if(!scope->expects_value)return MT_ERR_FORMAT;scope->expects_value=false;return MT_OK;}if(!scope->first){mt_status s=mt_json_raw(writer,",");if(s!=MT_OK)return s;}scope->first=false;return MT_OK;}
static mt_status mt_json_escape(mt_json_writer *writer,const char *text){mt_status s=mt_json_raw(writer,"\"");if(s!=MT_OK)return s;for(const unsigned char *p=(const unsigned char*)text;*p!=0u;++p){char small[8];switch(*p){case '"':s=mt_json_raw(writer,"\\\"");break;case '\\':s=mt_json_raw(writer,"\\\\");break;case '\b':s=mt_json_raw(writer,"\\b");break;case '\f':s=mt_json_raw(writer,"\\f");break;case '\n':s=mt_json_raw(writer,"\\n");break;case '\r':s=mt_json_raw(writer,"\\r");break;case '\t':s=mt_json_raw(writer,"\\t");break;default:if(*p<0x20u){int n=snprintf(small,sizeof(small),"\\u%04x",*p);s=n==6?mt_string_builder_append_n(&writer->builder,small,6u):MT_ERR_FORMAT;}else{s=mt_string_builder_append_n(&writer->builder,(const char*)p,1u);}break;}if(s!=MT_OK)return s;}return mt_json_raw(writer,"\"");}

mt_status mt_json_writer_init(mt_json_writer *writer){if(writer==NULL)return MT_ERR_ARGUMENT;writer->depth=0u;return mt_string_builder_init(&writer->builder);}
void mt_json_writer_dispose(mt_json_writer *writer){if(writer==NULL)return;mt_string_builder_dispose(&writer->builder);writer->depth=0u;}
const char *mt_json_writer_data(const mt_json_writer *writer){return writer==NULL?"":mt_string_builder_data(&writer->builder);}

mt_status mt_json_begin_object(mt_json_writer *writer){if(writer==NULL||writer->depth>=64u)return MT_ERR_ARGUMENT;mt_status s=mt_json_prefix(writer);if(s!=MT_OK)return s;s=mt_json_raw(writer,"{");if(s!=MT_OK)return s;writer->scopes[writer->depth++]=(mt_json_scope){MT_JSON_OBJECT,true,false};return MT_OK;}
mt_status mt_json_end_object(mt_json_writer *writer){if(writer==NULL||writer->depth==0u||writer->scopes[writer->depth-1u].kind!=MT_JSON_OBJECT||writer->scopes[writer->depth-1u].expects_value)return MT_ERR_FORMAT;--writer->depth;return mt_json_raw(writer,"}");}
mt_status mt_json_begin_array(mt_json_writer *writer){if(writer==NULL||writer->depth>=64u)return MT_ERR_ARGUMENT;mt_status s=mt_json_prefix(writer);if(s!=MT_OK)return s;s=mt_json_raw(writer,"[");if(s!=MT_OK)return s;writer->scopes[writer->depth++]=(mt_json_scope){MT_JSON_ARRAY,true,false};return MT_OK;}
mt_status mt_json_end_array(mt_json_writer *writer){if(writer==NULL||writer->depth==0u||writer->scopes[writer->depth-1u].kind!=MT_JSON_ARRAY)return MT_ERR_FORMAT;--writer->depth;return mt_json_raw(writer,"]");}
mt_status mt_json_key(mt_json_writer *writer,const char *key){if(writer==NULL||key==NULL||writer->depth==0u)return MT_ERR_ARGUMENT;mt_json_scope *scope=&writer->scopes[writer->depth-1u];if(scope->kind!=MT_JSON_OBJECT||scope->expects_value)return MT_ERR_FORMAT;mt_status s;if(!scope->first){s=mt_json_raw(writer,",");if(s!=MT_OK)return s;}scope->first=false;s=mt_json_escape(writer,key);if(s!=MT_OK)return s;s=mt_json_raw(writer,":");if(s==MT_OK)scope->expects_value=true;return s;}
mt_status mt_json_string(mt_json_writer *writer,const char *value){if(writer==NULL||value==NULL)return MT_ERR_ARGUMENT;mt_status s=mt_json_prefix(writer);return s==MT_OK?mt_json_escape(writer,value):s;}
mt_status mt_json_u32(mt_json_writer *writer,uint32_t value){if(writer==NULL)return MT_ERR_ARGUMENT;mt_status s=mt_json_prefix(writer);return s==MT_OK?mt_string_builder_append_u32(&writer->builder,value):s;}
mt_status mt_json_bool(mt_json_writer *writer,bool value){if(writer==NULL)return MT_ERR_ARGUMENT;mt_status s=mt_json_prefix(writer);return s==MT_OK?mt_json_raw(writer,value?"true":"false"):s;}
mt_status mt_json_null(mt_json_writer *writer){if(writer==NULL)return MT_ERR_ARGUMENT;mt_status s=mt_json_prefix(writer);return s==MT_OK?mt_json_raw(writer,"null"):s;}
