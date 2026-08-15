#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <wchar.h>

static void qemu_print_char(char c) {
  // Use QEMU's debugcon device
  __asm__ volatile("outb %0, $0xe9" ::"r"(c));
}

static void qemu_print_chars(const char* chars, size_t size) {
  for (size_t i = 0; i < size; i++) {
    qemu_print_char(chars[i]);
  }
}

static void qemu_print_unsigned_num(uint64_t num, uint8_t base) {
  static char digits[64];

  if (num == 0) {
    qemu_print_char('0');
    return;
  }

  size_t idx = sizeof(digits);
  while (num > 0) {
    idx--;
    size_t remainder = num % base;
    if (remainder < 10) {
      digits[idx] = remainder + '0';
    } else {
      digits[idx] = remainder + 'A' - 10;
    }
    num /= base;
  }

  qemu_print_chars(&digits[idx], sizeof(digits) - idx);
}

// TODO: Implement a real version of this function. This implementation contains
// bugs and buffer overflows.
unsigned long qemu_print(const char* fmt, ...) {
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
              qemu_print_unsigned_num(arg, 10);
              break;
            }
            case 'x': {
              uint64_t arg = va_arg(args, uint64_t);
              qemu_print_unsigned_num(arg, 16);
              break;
            }
            case 's': {
              wchar_t* arg = va_arg(args, wchar_t*);
              for (const wchar_t* arg_c = arg; *arg_c != '\0'; arg_c++) {
                // NOTE: This treats wide strings as regular strings
                qemu_print_char((char)*arg_c);
              }
              break;
            }
          }
          break;
        }
        case 'u': {
          uint32_t arg = va_arg(args, uint32_t);
          qemu_print_unsigned_num(arg, 10);
          break;
        }
        case 'x': {
          uint32_t arg = va_arg(args, uint32_t);
          qemu_print_unsigned_num(arg, 16);
          break;
        }
        case 's': {
          char* arg = va_arg(args, char*);
          for (const char* arg_c = arg; *arg_c != '\0'; arg_c++) {
            qemu_print_char(*arg_c);
          }
          break;
        }
      }
    } else {
      if (*c == '%' && *(c + 1) == '%') {
        c++;
      }

      qemu_print_char(*c);
    }
  }

  va_end(args);
  return 0;
}
