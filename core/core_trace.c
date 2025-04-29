#include "core_trace.h"

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

static void print_char(char c) {
  // Use QEMU's debugcon device
  asm volatile("outb %0, $0xe9" ::"r"(c));
}

static void print_digits(const char* digits, size_t size) {
  for (int i = size - 1; i >= 0; i--) {
    print_char(*(digits + i));
  }
}

static void print_unsigned_num(uint64_t num, uint8_t base) {
  static char digits[64];

  if (num == 0) {
    print_char('0');
    return;
  }

  size_t idx = 0;
  while (num > 0) {
    size_t remainder = num % base;
    if (remainder < 10) {
      digits[idx] = remainder + '0';
    } else {
      digits[idx] = remainder + 'A' - 10;
    }
    idx++;
    num /= base;
  }

  print_digits(digits, idx);
}

// TODO: Implement a real version of this function. This implementation contains
// bugs and buffer overflows.
void trace(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);

  for (const char* c = fmt; *c != '\0'; c++) {
    if (*c == '%' && *(c + 1) != '%' && *(c + 1) != '\0') {
      c++;
      switch (*c) {
        case 'l': {
          c++;
          switch (*c) {
            case 'u': {
              uint64_t arg = va_arg(args, uint64_t);
              print_unsigned_num(arg, 10);
              break;
            }
            case 'x': {
              uint64_t arg = va_arg(args, uint64_t);
              print_unsigned_num(arg, 16);
              break;
            }
          }
          break;
        }
        case 'u': {
          uint32_t arg = va_arg(args, uint32_t);
          print_unsigned_num(arg, 10);
          break;
        }
        case 'x': {
          uint32_t arg = va_arg(args, uint32_t);
          print_unsigned_num(arg, 16);
          break;
        }
        case 's': {
          char* arg = va_arg(args, char*);
          for (const char* arg_c = arg; *arg_c != '\0'; arg_c++) {
            print_char(*arg_c);
          }
          break;
        }
      }
    } else {
      if (*c == '%' && *(c + 1) == '%') {
        c++;
      }

      print_char(*c);
    }
  }

  va_end(args);
}
