#ifndef _INCLUDE_TRACE
#define _INCLUDE_TRACE

extern void trace(const char* fmt, ...);

#ifdef DEBUG
#define TRACE(fmt, ...) trace(fmt, ##__VA_ARGS__)
#else
#define TRACE(fmt, ...)
#endif

#endif
