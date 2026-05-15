#ifndef __R3_GRAPH_H__
#define __R3_GRAPH_H__

#include <include/libR3/ds/soa.h>
#include <include/libR3/math/math.h>

/**
 * Vertex layout:
 * vertexA  [<u64>]
 * vertexB  [<u64>]
 * index    [<u64>]
 * data     [stride]
 * 
 * Edge layout:
 * vertexA  [<u64>]
 * vertexB  [<u64>]
 * index    [<u64>]
 * data     [stride]
 * */

typedef struct R3Edge {
    u64 vertexA;
    u64 vertexB;
    u64 index;
    ptr data;
} R3Edge;

typedef struct R3Vertex {
    u64 vertexA;
    u64 vertexB;
    u64 index;
    ptr data;
} R3Vertex;

typedef struct R3GraphDesc {
    u64 vertexStride;
    u64 edgeStride;
    u32 vertexMax;
    u32 edgeMax;
} R3GraphDesc;

typedef struct R3Graph {
    R3SOA vertices;
    R3SOA edges;
} R3Graph;

R3_PUBLIC_API R3Result r3NewGraph(R3GraphDesc desc, R3Graph* graph);
R3_PUBLIC_API R3Result r3DelGraph(R3Graph* graph);

R3_PUBLIC_API R3Result r3SetVertex(Vec2 vertex, R3Graph* graph);
R3_PUBLIC_API R3Vertex r3GetVertex(Vec2 vertex, R3Graph* graph);
R3_PUBLIC_API R3Result r3RemVertex(Vec2 vertex, R3Graph* graph);

#endif // __R3_GRAPH_H__
