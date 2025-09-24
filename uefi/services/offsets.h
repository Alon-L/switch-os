#ifndef _HOOKS_OFFSETS
#define _HOOKS_OFFSETS

// The header contains the parameters of the hook, passed by the hook loader.
// It is contained in the same section as the `.data` and `.rodata` sections,
// since they all have the same permissions (RW).
//
// This can't be 0, since there has to be place for the PHDRs, otherwise the
// linker complains.
#define HOOK_HEADER_OFFSET (0x1000)
#define HOOK_HEADER_SIZE (0x1000)

// The hook's entry is the first part of the hook's execution flow.
// It is contained in the same section as the `.text` section.
//
// Since it has execuable permissions, it can't be in the same page as the
// header, otherwise the page would have RWX permissions, and the linker would
// complain.
#define HOOK_ENTRY_OFFSET (0x2000)

// The first section in the output ELF file.
#define FIRST_SECTION_OFFSET HOOK_HEADER_OFFSET

// The raw binary files only contain the header and entry sections,
// so their offsets are shifted back.
#define RAW_HOOK_HEADER_OFFSET (HOOK_HEADER_OFFSET - FIRST_SECTION_OFFSET)
#define RAW_HOOK_ENTRY_OFFSET (HOOK_ENTRY_OFFSET - FIRST_SECTION_OFFSET)

#endif
