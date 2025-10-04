#ifndef _INCLUDE_ERROR
#define _INCLUDE_ERROR

#include <stdint.h>

#include "trace.h"

typedef enum {
  SUCCESS = 0,
  ERR_INVALID = 1,
} err_t;

#define CHECK(expr) CHECK_TRACE((expr), "Error on %s:%u\n", __FILE__, (uint32_t)__LINE__);

#define CHECK_RETHROW(expr) CHECK_RETHROW_TRACE(expr, "Rethrown error on %s:%u\n", __FILE__, (uint32_t)__LINE__)

#define CHECK_RETHROW_TRACE(expr, fmt, ...) \
  do {                                      \
    err_t _err = (expr);                    \
    if (!IS_SUCCESS(_err)) {                \
      err = _err;                           \
      TRACE(fmt, ##__VA_ARGS__);            \
      goto cleanup;                         \
    }                                       \
  } while (0)

#define CHECK_RETHROW_SILENT(expr) \
  do {                             \
    err_t _err = (expr);           \
    if (!IS_SUCCESS(_err)) {       \
      err = _err;                  \
      goto cleanup;                \
    }                              \
  } while (0)

#define CHECK_TRACE(expr, fmt, ...) \
  do {                              \
    if (!(expr)) {                  \
      err = ERR_INVALID;            \
      TRACE(fmt, ##__VA_ARGS__);    \
      goto cleanup;                 \
    }                               \
  } while (0)

#define CHECK_SILENT(expr) \
  do {                     \
    if (!(expr)) {         \
      err = ERR_INVALID;   \
      goto cleanup;        \
    }                      \
  } while (0)

#define IS_SUCCESS(expr) ((expr) == SUCCESS)

#define CHECK_FAIL() CHECK(0)

#define CHECK_FAIL_SILENT() CHECK_SILENT(0)

#define CHECK_FAIL_TRACE(fmt, ...) CHECK_TRACE(0, fmt, ##__VA_ARGS__)

#endif
