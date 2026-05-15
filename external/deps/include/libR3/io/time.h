#ifndef __R3_IO_TIME_H__
#define __R3_IO_TIME_H__

#include <include/libR3/r3def.h>

typedef struct {
    i32 hour;
    i32 minute;
    i32 second;
    i32 millisecond;
} R3LocalTime;

typedef struct R3Timer {
    f64 start;
    f64 stop;
    f64 sec;
    f64 ms;
} R3Timer;

R3_PUBLIC_API R3LocalTime r3GetLocalTime(none);

R3_PUBLIC_API void r3ClockSleepMs(u32 ms);
R3_PUBLIC_API R3Timer r3StartTimer(none);
R3_PUBLIC_API none r3StopTimer(R3Timer* timer);

#endif // __R3_IO_TIME_H__
