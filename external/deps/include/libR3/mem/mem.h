#ifndef __R3_MEM_BASE_H__
#define __R3_MEM_BASE_H__

#include <include/libR3/r3def.h>

R3_PUBLIC_API ptr r3AllocMemory(u64 size);
R3_PUBLIC_API u64 r3MemorySize(ptr memory);
R3_PUBLIC_API R3Result r3FreeMemory(ptr memory);
R3_PUBLIC_API ptr r3ReallocMemory(u64 size, ptr memory);

R3_PUBLIC_API R3Result r3AssignMemory(ptr source, ptr dest);
R3_PUBLIC_API R3Result r3SetMemory(u64 bytes, u8 value, ptr memory);
R3_PUBLIC_API R3Result r3ReadMemory(u64 bytes, ptr source, ptr dest);
R3_PUBLIC_API R3Result r3MoveMemory(u64 bytes, ptr source, ptr dest);
R3_PUBLIC_API R3Result r3WriteMemory(u64 bytes, ptr source, ptr dest);
R3_PUBLIC_API R3Result r3CompareMemory(u64 bytes, ptr mem1, ptr mem2);

#endif /* __R3_MEM_BASE_H__ */