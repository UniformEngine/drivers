#ifndef __R3_MEM_BLOB_H__
#define __R3_MEM_BLOB_H__

#include <include/libR3/r3def.h>

typedef struct R3Blob {
    u64 start;
    u64 stop;
    u64 size;
} R3Blob;

typedef struct R3BlobAllocator {
    u64 size;
    u64 used;
    u64 bump;
    u64 stop;
    ptr start;
    R3Blob* free;
} R3BlobAllocator;

R3_PUBLIC_API R3Result r3DelBlobAllocator(R3BlobAllocator* alloc);
R3_PUBLIC_API R3Result r3NewBlobAllocator(u64 size, R3BlobAllocator* alloc);

R3_PUBLIC_API R3Blob r3BlobAlloc(u64 size, R3BlobAllocator* alloc);
R3_PUBLIC_API R3Result r3BlobFree(R3Blob* blob, R3BlobAllocator* alloc);

R3_PUBLIC_API R3Result r3BlobSet(R3Blob* blob, u8 value, R3BlobAllocator* alloc);
R3_PUBLIC_API R3Result r3BlobRead(R3Blob* blob, ptr data, R3BlobAllocator* alloc);
R3_PUBLIC_API R3Result r3BlobWrite(R3Blob* blob, ptr data, R3BlobAllocator* alloc);

#endif // __R3_MEM_BLOB_H__