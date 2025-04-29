#ifndef _INCLUDE_TRACE
#define _INCLUDE_TRACE

#ifdef TRACE_DEBUG
extern __attribute__((format(printf, 1, 2))) void trace(const char* fmt, ...);

#define TRACE(fmt, ...) trace(fmt, ##__VA_ARGS__)
#else
#define TRACE(fmt, ...)
#endif

#endif
