#ifndef _INCLUDE_ERROR
#define _INCLUDE_ERROR

#include "trace.h"

typedef enum {
  SUCCESS = 0,
  ERROR = 1,
} err_t;

#define CHECK(expr) CHECK_TRACE((expr), "Error on %d:%d", __FILE__, __LINE__);

#define CHECK_RETHROW(expr)                                 \
  do {                                                      \
    err_t _err = (expr);                                    \
    if (!IS_SUCCESS(_err)) {                                \
      err = _err;                                           \
      TRACE("Rethrown error on %s:%d", __FILE__, __LINE__); \
      goto cleanup;                                         \
    }                                                       \
  } while (0)

#define CHECK_TRACE(expr, fmt, ...) \
  do {                              \
    if (!(expr)) {                  \
      err = ERROR;                  \
      TRACE(fmt, ##__VA_ARGS__);    \
      goto cleanup;                 \
    }                               \
  } while (0)

#define IS_SUCCESS(expr) ((expr) == SUCCESS)

#endif
