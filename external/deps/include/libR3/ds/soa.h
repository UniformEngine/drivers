#ifndef __R3_DS_STRUCT_H__
#define __R3_DS_STRUCT_H__

#include <include/libR3/r3def.h>

typedef struct R3SOADesc {
    u64* strides;
    u32 rows;
    u32 cols;
} R3SOADesc;

typedef struct R3SOAView {
    ptr* rows;
    u32 count;
} R3SOAView;

// A convenience macro for bounded access into a `R3SOAView`.
// NOTE: incurs ~25-45ms overhead over inline access. (e.g ptr row = view->rows[row])
#define R3SOAViewField(view, row) ((R3SOAView*)view)->rows[(row) >= ((R3SOAView*)view)->count ? (row) - ((R3SOAView*)view)->count : (row)]

typedef struct R3SOA { ptr rows; } R3SOA;
typedef none (*R3SOAProcess)(u32 index, ptr user, R3SOAView* view);

R3_PUBLIC_API R3Result r3DelSOA(R3SOA* soa);
R3_PUBLIC_API R3Result r3NewSOA(R3SOADesc desc, R3SOA* soa);

R3_PUBLIC_API u64 r3SOASize(R3SOA* soa);
R3_PUBLIC_API u64 r3SOARows(R3SOA* soa);
R3_PUBLIC_API u64 r3SOACols(R3SOA* soa);
R3_PUBLIC_API u64 r3SOAStride(u32 row, R3SOA* soa);

R3_PUBLIC_API R3SOAView* r3ViewSOA(R3SOA* soa);
R3_PUBLIC_API R3Result r3IterSOA(u64 cycles, R3SOAProcess proc, ptr user, R3SOA* soa);

R3_PUBLIC_API R3Result r3RemSOA(u32 row, u32 col, R3SOA* soa);
R3_PUBLIC_API R3Result r3GetSOA(u32 row, u32 col, ptr value, R3SOA* soa);
R3_PUBLIC_API R3Result r3SetSOA(u32 row, u32 col, ptr value, R3SOA* soa);

#endif // __R3_DS_STRUCT_H__