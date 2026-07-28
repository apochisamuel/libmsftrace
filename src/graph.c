#include "semantic.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct mt_graph_edge { uint32_t from; uint32_t to; uint32_t weight; } mt_graph_edge;
typedef struct mt_graph { mt_graph_edge *edges; size_t count; size_t capacity; uint32_t vertex_count; } mt_graph;

mt_status mt_graph_init(mt_graph *graph, uint32_t vertex_count) {
    if (graph == NULL) return MT_ERR_ARGUMENT;
    graph->edges=NULL; graph->count=0u; graph->capacity=0u; graph->vertex_count=vertex_count; return MT_OK;
}

void mt_graph_dispose(mt_graph *graph) {
    if(graph==NULL)return; free(graph->edges); graph->edges=NULL; graph->count=0u; graph->capacity=0u; graph->vertex_count=0u;
}

static mt_status mt_graph_reserve(mt_graph *graph,size_t capacity) {
    if(capacity<=graph->capacity)return MT_OK; size_t next=graph->capacity==0u?16u:graph->capacity;
    while(next<capacity){if(next>SIZE_MAX/2u){next=capacity;break;}next*=2u;}
    if(next>SIZE_MAX/sizeof(mt_graph_edge))return MT_ERR_NOMEM;
    mt_graph_edge *edges=(mt_graph_edge*)realloc(graph->edges,next*sizeof(mt_graph_edge)); if(edges==NULL)return MT_ERR_NOMEM;
    graph->edges=edges;graph->capacity=next;return MT_OK;
}

mt_status mt_graph_add_edge(mt_graph *graph,uint32_t from,uint32_t to,uint32_t weight) {
    if(graph==NULL||from>=graph->vertex_count||to>=graph->vertex_count)return MT_ERR_ARGUMENT;
    mt_status status=mt_graph_reserve(graph,graph->count+1u);if(status!=MT_OK)return status;
    graph->edges[graph->count++]=(mt_graph_edge){from,to,weight};return MT_OK;
}

mt_status mt_graph_topological_order(const mt_graph *graph,uint32_t *output,size_t capacity,size_t *out_count) {
    if(graph==NULL||output==NULL||out_count==NULL)return MT_ERR_ARGUMENT;
    if(capacity<graph->vertex_count)return MT_ERR_NOMEM;
    uint32_t *degree=(uint32_t*)calloc(graph->vertex_count,sizeof(uint32_t));
    uint32_t *queue=(uint32_t*)calloc(graph->vertex_count,sizeof(uint32_t));
    if((degree==NULL||queue==NULL)&&graph->vertex_count!=0u){free(degree);free(queue);return MT_ERR_NOMEM;}
    for(size_t i=0u;i<graph->count;++i)++degree[graph->edges[i].to];
    size_t head=0u,tail=0u,written=0u;for(uint32_t v=0u;v<graph->vertex_count;++v)if(degree[v]==0u)queue[tail++]=v;
    while(head<tail){uint32_t v=queue[head++];output[written++]=v;for(size_t i=0u;i<graph->count;++i){if(graph->edges[i].from!=v)continue;uint32_t to=graph->edges[i].to;if(--degree[to]==0u)queue[tail++]=to;}}
    free(degree);free(queue);*out_count=written;return written==graph->vertex_count?MT_OK:MT_ERR_FORMAT;
}

mt_status mt_graph_shortest_paths(const mt_graph *graph,uint32_t source,uint64_t *distances,size_t capacity) {
    if(graph==NULL||distances==NULL||source>=graph->vertex_count)return MT_ERR_ARGUMENT;
    if(capacity<graph->vertex_count)return MT_ERR_NOMEM;
    bool *visited=(bool*)calloc(graph->vertex_count,sizeof(bool));if(visited==NULL&&graph->vertex_count!=0u)return MT_ERR_NOMEM;
    for(uint32_t i=0u;i<graph->vertex_count;++i)distances[i]=UINT64_MAX;distances[source]=0u;
    for(uint32_t step=0u;step<graph->vertex_count;++step){uint32_t best=UINT32_MAX;for(uint32_t v=0u;v<graph->vertex_count;++v)if(!visited[v]&&distances[v]!=UINT64_MAX&&(best==UINT32_MAX||distances[v]<distances[best]))best=v;
        if(best==UINT32_MAX)break;visited[best]=true;for(size_t i=0u;i<graph->count;++i){if(graph->edges[i].from!=best)continue;uint32_t to=graph->edges[i].to;uint64_t candidate=distances[best]+graph->edges[i].weight;if(candidate<distances[to])distances[to]=candidate;}}
    free(visited);return MT_OK;
}

mt_status mt_graph_reachable(const mt_graph *graph,uint32_t source,bool *output,size_t capacity) {
    if(graph==NULL||output==NULL||source>=graph->vertex_count||capacity<graph->vertex_count)return MT_ERR_ARGUMENT;
    memset(output,0,graph->vertex_count*sizeof(bool));uint32_t *stack=(uint32_t*)malloc(graph->vertex_count*sizeof(uint32_t));if(stack==NULL&&graph->vertex_count!=0u)return MT_ERR_NOMEM;
    size_t count=0u;stack[count++]=source;output[source]=true;while(count!=0u){uint32_t v=stack[--count];for(size_t i=0u;i<graph->count;++i){if(graph->edges[i].from!=v)continue;uint32_t to=graph->edges[i].to;if(!output[to]){output[to]=true;stack[count++]=to;}}}free(stack);return MT_OK;
}
