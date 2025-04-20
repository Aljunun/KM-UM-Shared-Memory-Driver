#include "includes.h"

bool WriteRegistryDword(HKEY hKeyRoot, LPCWSTR SubKey, LPCWSTR ValueName, DWORD Data);
bool WriteRegistryQword(HKEY hKeyRoot, LPCWSTR SubKey, LPCWSTR ValueName, DWORD_PTR Data);
bool CleanRegistry();