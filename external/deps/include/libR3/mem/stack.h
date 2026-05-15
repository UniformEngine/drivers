#ifndef __R3_MEM_STACK_H__
#define __R3_MEM_STACK_H__

/*
    Stack Allocator
    ~~~~~~~~~~~~~~
    The stack allocator is the simplest and fastest form of allocation.
    It linearly advances a single offset through a preallocated buffer.
    Individual frees are only valid if performed in strict LIFO order.
    This prevents offset/buffer corruption and guarantees that memory
    usage is always contiguous.

    Design Notes
    ~~~~~~~~~~~~
    * Each allocation prepends a 64-bit size header to the returned block.
    * The allocator maintains its own metadata buffer for bookkeeping.
    * Deallocation validates LIFO order before rolling back the offset.
    * No dynamic OS allocations occur after initialization.

    Changelog
    ~~~~~~~~~
    [@zafflins 10/18/25 v1.0]
    ~ Initial implementation of the stack allocator.
               Added `r3NewStackAllocator()`, `r3DelStackAllocator()`.
               Added `R3Allocator` interface and function pointers.

    [@zafflins 10/18/25 v1.0] 
    Stack Allocator Example
    ```c
        R3Allocator a = {0};
        r3NewStackAllocator(1024, &a);

        f32* v = a.alloc(sizeof(f32) * 4, &a);
        ...
        a.free((ptr*)&v, &a);

        r3DelStackAllocator(&a);
    ```
*/

#include <include/libR3/mem/mem.h>

typedef struct R3StackAllocator {
    ptr meta;                 // allocator metadata region
    ptr base;                 // Base memory region used for allocations
    ptr (*alloc)(u64 size, struct R3StackAllocator* alloc);
    R3Result (*free)(ptr data, struct R3StackAllocator* alloc);
} R3StackAllocator;

R3_PUBLIC_API R3Result r3NewStackAllocator(u64 size, R3StackAllocator* alloc);
R3_PUBLIC_API R3Result r3DelStackAllocator(R3StackAllocator* alloc);

#endif /* __R3_MEM_STACK_H__ */