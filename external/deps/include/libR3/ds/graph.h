#ifndef __R3_GRAPH_H__
#define __R3_GRAPH_H__

#include <include/libR3/r3def.h>

typedef struct R3DAGBuildDesc {
    u32* targets;
    u32* weights;
    u32 count;
    u32 from;
} R3DAGBuildDesc;

typedef struct R3DAG { ptr data; } R3DAG;

R3_PUBLIC_API R3Result r3NewDAG(u32 vmax, u32 emax, u64 stride, R3DAG* graph);
R3_PUBLIC_API R3Result r3DelDAG(R3DAG* graph);
R3_PUBLIC_API none r3PrintDAG(R3DAG* graph);

R3_PUBLIC_API u32 r3DAGEdges(R3DAG* graph);
R3_PUBLIC_API u64 r3DAGStride(R3DAG* graph);
R3_PUBLIC_API u32 r3DAGVertices(R3DAG* graph);
R3_PUBLIC_API u32* r3DAGIndegree(R3DAG* graph);

R3_PUBLIC_API u32* r3SortDAG(R3DAG* graph);
R3_PUBLIC_API u32* r3DFSDAG(R3DAG* graph);

R3_PUBLIC_API u32 r3NewDAGVertex(ptr data, R3DAG* graph);
R3_PUBLIC_API R3Result r3BuildDAGVertex(R3DAGBuildDesc desc, R3DAG* graph);
R3_PUBLIC_API ptr r3GetDAGVertex(u32 vertex, R3DAG* graph);
R3_PUBLIC_API R3Result r3SetDAGVertex(u32 vertex, ptr data, R3DAG* graph);

#endif // __R3_GRAPH_H__
