#include "kernel.h"
#include <thread>
#include <Windows.h>
#include "misc.h"
  

std::mutex& getMutex() {
    static std::mutex mtx;
    return mtx;
}
 
// initialisation for the communication with kernel mode
bool initCommunication() {
	req = nullptr;
    req = (Requests*)VirtualAlloc(NULL, sizeof(Requests),
        MEM_COMMIT | MEM_RESERVE,
        PAGE_READWRITE);
    if (req == nullptr) {
        std::cout << "[-] Failed to allocate memory for queue" << std::endl;
        return false;
    }

    memset(req, 0, sizeof(Requests));
	req->g_pid = getprocessid(L"DriverTester.exe");
	req->Op = op_a;

    std::cout << "[+] Que initialized at address: " << std::hex << reinterpret_cast<uintptr_t>(req) << std::endl;
    // Way to send data to kernel mode
    if (!WriteRegistryDword(HKEY_LOCAL_MACHINE, L"", L"oPid", GetCurrentProcessId())) { return false; }
    if (!WriteRegistryQword(HKEY_LOCAL_MACHINE, L"", L"oAddr", reinterpret_cast<DWORD_PTR>(req))) {
        VirtualFree(req, 0, MEM_RELEASE);
        return false;
    }     
  
 
  
      InterlockedExchange(&req->Ready, 1);

    while (InterlockedCompareExchange(&req->uready, 0, 1) != 1) {
        _mm_pause();   
    }
    InterlockedExchange(&req->uready, 0);

	int buffer = (int)req->buffer;
	std::cout << "buffer: "  << std::dec<< buffer << std::endl;

 
    return true;
}

void exitKernel() {
	req->Op = op_exit;  
	req->InUse = 1;
    InterlockedExchange(&req->Ready, 1);
	Sleep(1000);
 
  
     std::cout << "[+] Goodbye kernel !" << std::endl;
}
