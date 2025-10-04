#ifndef _INCLUDE_UTILS_H
#define _INCLUDE_UTILS_H

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

#ifndef MIN
#define MIN(a, b)           \
  ({                        \
    __typeof__(a) _a = (a); \
    __typeof__(b) _b = (b); \
    _a < _b ? _a : _b;      \
  })
#endif

#ifndef MAX
#define MAX(a, b)           \
  ({                        \
    __typeof__(a) _a = (a); \
    __typeof__(b) _b = (b); \
    _a > _b ? _a : _b;      \
  })
#endif

#ifndef ALIGN_UP
#define ALIGN_UP(x, align_to) (((x) + ((align_to) - 1)) & ~((align_to) - 1))
#endif

#ifndef UNUSED_PARAM
#define UNUSED_PARAM __attribute__((unused))
#endif

#endif
