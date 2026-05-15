#ifndef __R3_DS_TREE_H__
#define __R3_DS_TREE_H__

/**
 * nodes, stored in an R3SOA, have a u64 size a u64 data, u64 left, and u64 right field
 * where data is an address somewhere in the tree data field with size guarding access
 * and left/right are addresses somewhere in the tree data field to child nodes
 * 
 */

#include <include/libR3/ds/soa.h>

typedef struct R3Tree {
    R3SOA nodes;
    ptr data;
} R3Tree;

#endif // __R3_DS_TREE_H__