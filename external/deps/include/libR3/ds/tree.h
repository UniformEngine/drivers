#ifndef __R3_DS_TREE_H__
#define __R3_DS_TREE_H__

#include <include/libR3/ds/soa.h>

typedef struct R3TreeNode {
    R3Handle children[8];
    ptr data;
} R3TreeNode;

typedef struct R3TreeHeader {
    R3Handle root;
    u64 stride;
    u32 factor;
    u32 nodes;
} R3TreeHeader;

// internally the data pointer is to a memory blob the size of (R3TreeHeader + R3SOA)
// the R3SOA is then used to store all node children, and data contiguously,
// accessible via a single R3Handle.
typedef struct R3Tree { ptr data; } R3Tree;

R3_PUBLIC_API R3Result r3NewTree(u64 stride, u32 factor, R3Tree* tree);
R3_PUBLIC_API R3Result r3DelTree(R3Tree* tree);

#endif // __R3_DS_TREE_H__
