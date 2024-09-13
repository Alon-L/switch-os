#ifndef _INCLUDE_TRACE
#define _INCLUDE_TRACE

extern void trace(char* fmt, ...);

#define TRACE(fmt, ...) trace(fmt, ##__VA_ARGS__)

#endif
