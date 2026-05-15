#ifndef __R3_MEM_BLOCK_H__
#define __R3_MEM_BLOCK_H__

/*
    Block Allocator
    ~~~~~~~~~~~~
    The block allocator provides a simple and fast allocation algorithm,
    along with a much more lenient deallocation model, allowing calls to `free()`
    to take place in no specific order, invalidating that block only.

    This is ideal for storage of any number of similar types, of any lifetime. (e.g runtime resources, persistent assets).

    Design Notes
    ~~~~~~~~~~~~
    * Allocations involve simple pointer arithmetic and bitwise operations marking blocks as unused for later reuse.
    * An 8-bit header stores each block's index for debugging or validation.
    * Calling `free()` invalidates the block pointer passed in, filling the block with `-1`'s.
    * No per-allocation deallocation occurs, just simple pointer arithmetic and bitwise operations.
    * No dynamic OS allocation occurs after initialization -- R3 block allocators allocate no more than 64 fixed size blocks.
    
    Changelog
    ~~~~~~~~~
    [@zafflins 10/23/25 v1.0]
    ~ Initial implementation of the block allocator.
    Added `r3NewBlockAllocator()`, `r3DelBlockAllocator()`.

    [@zafflins 10/23/25 v1.0]
    Block Allocator Example
    ~~~~~~~~~~~~
    ```c
        R3Allocator a = {0};
        r3NewBlockAllocator((sizeof(f32) * 3) * 3, &a);    // each block is an array of 3 vec3f
        
        f32** vec3v1 = a.alloc(&a);  // allocate a single block of vec3f arrays
        ...

        a.free(&vec3v1, &a);
        r3DelBlockAllocator(&a);
    ```
*/

#include <include/libR3/mem/mem.h>

typedef struct R3BlockAllocator {
    ptr meta;                 // allocator metadata region
    ptr base;                 // Base memory region used for allocations
    ptr (*alloc)(struct R3BlockAllocator* alloc);
    R3Result (*free)(ptr data, struct R3BlockAllocator* alloc);
} R3BlockAllocator;

R3_PUBLIC_API R3Result r3NewBlockAllocator(u32 bstride, R3BlockAllocator* alloc);
R3_PUBLIC_API R3Result r3DelBlockAllocator(R3BlockAllocator* alloc);

#endif /* __R3_MEM_BLOCK_H__ */