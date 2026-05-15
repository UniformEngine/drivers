#ifndef __R3_MEM_ARENA_H__
#define __R3_MEM_ARENA_H__

/*
    Arena Allocator
    ~~~~~~~~~~~~
    The arena allocator provides a linear, monotonic allocation model similar
    to the stack allocator, but without LIFO restrictions. Individual frees are
    ignored — the entire arena is reset in one operation via `free()`.

    This makes it ideal for short-lived or batch allocations where the lifetime
    of all objects is shared (e.g., per-frame systems, parser scratch space).

    Design Notes
    ~~~~~~~~~~~~
    * Allocations advance a single offset through a preallocated buffer.
    * A 64-bit header stores each allocation’s size for debugging or validation.
    * Calling `free()` resets the arena’s offset to zero and reclaims all memory.
    * No per-allocation deallocation occurs; reset is all-or-nothing.
    * No dynamic OS allocations occur after initialization.

    Changelog
    ~~~~~~~~~
    [@zafflins 10/21/25 v1.0]
    ~ Initial implementation of the arena allocator.
    Added `r3NewArenaAllocator()`, `r3DelArenaAllocator()`.
    
    [@zafflins 10/21/25 v1.0]
    Arena Allocator Example
    ~~~~~~~~~~~~
    ```c
        R3Allocator a = {0};
        r3NewArenaAllocator(4096, &a);

        char* temp1 = a.alloc(128, &a);
        char* temp2 = a.alloc(512, &a);
        ...
        
        a.free(NULL, &a);  // resets entire arena and issues a call to `memset` filling the arena with `-1`'s (temp1 and temp2 are still valid)
        r3DelArenaAllocator(&a); (temp1 and temp2 are now invalid)
    ```
*/

#include <include/libR3/mem/mem.h>

typedef struct R3ArenaAllocator {
    ptr meta;                 // allocator metadata region
    ptr base;                 // Base memory region used for allocations
    ptr (*alloc)(u64 size, struct R3ArenaAllocator* alloc);
    R3Result (*free)(ptr data, struct R3ArenaAllocator* alloc);
} R3ArenaAllocator;

R3_PUBLIC_API R3Result r3NewArenaAllocator(u64 size, R3ArenaAllocator* alloc);
R3_PUBLIC_API R3Result r3DelArenaAllocator(R3ArenaAllocator* alloc);

#endif /* __R3_MEM_ARENA_H__ */