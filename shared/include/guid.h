#ifndef _INCLUDE_GUID
#define _INCLUDE_GUID

#include <stdint.h>

struct guid {
  uint32_t data1;
  uint16_t data2;
  uint16_t data3;
  uint8_t data4[8];
};

// Arbitrary GUID that the running kernel needs to use in order to communicate with us and send commands.
#define EFI_COMMANDS_GUID {0xe21c66ed, 0xbc2e, 0x4d30, 0xac, 0x5e, 0x1b, 0x96, 0x80, 0x02, 0xc5, 0x41}

#endif
