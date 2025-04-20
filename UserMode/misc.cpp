#include "misc.h"

float randomFloat()
{
    return (float)(rand()) / (float)(rand());
}

int getprocessid(const wchar_t* process_name) {
    PROCESSENTRY32 processentry;
    HANDLE snapshot_handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (snapshot_handle == INVALID_HANDLE_VALUE)
        return 0;

    processentry.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(snapshot_handle, &processentry)) {
        do {
            if (wcscmp(process_name, processentry.szExeFile) == 0) {
                CloseHandle(snapshot_handle);
                return processentry.th32ProcessID;
            }
        } while (Process32Next(snapshot_handle, &processentry));
    }

    CloseHandle(snapshot_handle);
    return 0;
}