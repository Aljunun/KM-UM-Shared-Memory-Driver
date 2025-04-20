#include <ntifs.h>
#include <windef.h>
#include <cstdint>
 
#define printfx(text, ...)		(DbgPrintEx(0, 0, text, ##__VA_ARGS__))

