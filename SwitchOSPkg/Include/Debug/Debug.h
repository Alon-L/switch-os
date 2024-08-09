#ifndef _SWITCH_O_S_PKG_DEBUG_H
#define _SWITCH_O_S_PKG_DEBUG_H

#include <Library/UefiLib.h>

#define _SWITCH_O_S_PKG_DEBUG

#ifdef _SWITCH_O_S_PKG_DEBUG
#define DbgPrintAscii( ...) AsciiPrint( __VA_ARGS__ )
#define DbgPrintUnicode(...) Print(__VA_ARGS__ )
#else
#define DbgPrintAscii(...)
#define DbgPrintUnicode(...)
#endif


#endif
