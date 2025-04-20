#include "registry.h"

bool WriteRegistryDword(HKEY hKeyRoot, LPCWSTR SubKey, LPCWSTR ValueName, DWORD Data) {
    HKEY hKey;
    LONG result;

    result = RegOpenKeyEx(hKeyRoot, SubKey, 0, KEY_WRITE, &hKey);
    if (result != ERROR_SUCCESS) {
        std::wcerr << L"Failed to open registry key: " << SubKey << std::endl;
        return false;
    }

    result = RegSetValueEx(hKey, ValueName, 0, REG_DWORD, reinterpret_cast<const BYTE*>(&Data), sizeof(Data));
    if (result != ERROR_SUCCESS) {
        std::wcerr << L"Failed to write DWORD to registry: " << ValueName << std::endl;
        RegCloseKey(hKey);
        return false;
    }

    RegCloseKey(hKey);
    return true;
}

bool WriteRegistryQword(HKEY hKeyRoot, LPCWSTR SubKey, LPCWSTR ValueName, DWORD_PTR Data) {
    HKEY hKey;
    LONG result;

    result = RegOpenKeyEx(hKeyRoot, SubKey, 0, KEY_WRITE, &hKey);
    if (result != ERROR_SUCCESS) {
        std::wcerr << L"Failed to open registry key: " << SubKey << std::endl;
        return false;
    }

    result = RegSetValueEx(hKey, ValueName, 0, REG_QWORD, reinterpret_cast<const BYTE*>(&Data), sizeof(Data));
    if (result != ERROR_SUCCESS) {
        std::wcerr << L"Failed to write QWORD to registry: " << ValueName << std::endl;
        RegCloseKey(hKey);
        return false;
    }

    RegCloseKey(hKey);
    return true;
}

bool CleanRegistry() {
    HKEY hKey;
    LONG result;

    result = RegDeleteKeyValueA(HKEY_LOCAL_MACHINE, "", "oPid");
    if (result != ERROR_SUCCESS) {
        std::cout << result << std::endl;
        return false;
    }

    result = RegDeleteKeyValueA(HKEY_LOCAL_MACHINE, "", "oAddr");
    if (result != ERROR_SUCCESS) {
        std::cout << result << std::endl;
        return false;
    }

    return true;
}
