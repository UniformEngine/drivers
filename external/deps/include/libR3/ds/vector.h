#ifndef __R3_DS_VECTOR_H__
#define __R3_DS_VECTOR_H__

#include <include/libR3/r3def.h>

typedef struct R3Vector {
    u64* pool;
    u64* htoi;  // [handle] = idx
    u64* itoh;  // [idx] = handle
    ptr data;

} R3Vector;

R3_PUBLIC_API R3Result r3NewVector(u64 slots, u64 stride, R3Vector* vector);
R3_PUBLIC_API R3Result r3DelVector(R3Vector* vector);

R3_PUBLIC_API R3Handle r3PushVector(ptr value, R3Vector* vector);
R3_PUBLIC_API R3Result r3PullVector(R3Handle id, R3Vector* vector);
R3_PUBLIC_API R3Result r3GetVector(u64 id, ptr value, R3Vector* vector);

#endif // __R3_DS_VECTOR_H__