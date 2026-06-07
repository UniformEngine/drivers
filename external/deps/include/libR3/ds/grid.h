#ifndef __R3_DS_GRID_H__
#define __R3_DS_GRID_H__

#include <include/libR3/r3def.h>
#include <include/libR3/mem/arena.h>
#include <include/libR3/math/math.h>

typedef struct R3Grid { ptr data; } R3Grid;

R3_PUBLIC_API R3Result r3DelGrid(R3Grid* grid);
R3_PUBLIC_API Vec2 r3ToGridPos(Vec2 xy, R3Grid* grid);
R3_PUBLIC_API Vec2 r3FromGridPos(Vec2 xy, R3Grid* grid);
R3_PUBLIC_API R3Result r3NewGrid(Vec2 cellWH, u64 cellStride, Vec2 gridWH, R3Grid* grid);

R3_PUBLIC_API R3Result r3RemGrid(Vec2 xy, R3Grid* grid);
R3_PUBLIC_API R3Result r3SetGrid(Vec2 xy, ptr data, R3Grid* grid);
R3_PUBLIC_API R3Result r3GetGrid(Vec2 xy, ptr data, R3Grid* grid);

#endif // __R3_DS_GRID_H__
