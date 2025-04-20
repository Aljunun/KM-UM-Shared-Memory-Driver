#include "structs.h"
#include "registry.h"
#include <chrono>
#include <thread>
#include <mutex>
std::mutex& getMutex();
inline Requests* req = nullptr;

  bool initCommunication();
 
 
 template <typename T>
T read(uintptr_t addr) {
     std::cout << "the begining of read " << std::endl;
	 req->InUse = 1; // Mark as processing
    req->Op = op_r;
    req->TargetAddress = reinterpret_cast<PVOID>(addr);
    req->Size = sizeof(T);
    std::cout << "[+] Read request sent to kernel" << std::endl;
    InterlockedExchange(&req->Ready, 1);

    // Wait for the kernel to signal completion
    std::cout << "[+] Waiting for kernel response" << std::endl;
    auto start = std::chrono::steady_clock::now();
    while (InterlockedCompareExchange(&req->uready, 0, 1) != 1) {
        if (std::chrono::steady_clock::now() - start > std::chrono::seconds(1)) {
            std::cout << "[-] Timeout\n";
            return T{}; // Return default on timeout
        }
        _mm_pause();
    }

    // Copy the data directly from the buffer field
    T value;
    memcpy(&value, &req->buffer, sizeof(T));
    std::cout << "reset" << std::endl;
    InterlockedExchange(&req->uready, 0); // Reset
}

// Write template
template <typename T>
void write(uintptr_t addr, T value) {
 
     req->Op = 20;        
    req->TargetAddress = reinterpret_cast<PVOID>(addr);
    req->Size = sizeof(T);
      if (req->buffer) {
        VirtualFree(req->buffer, 0, MEM_RELEASE);
    }
    req->buffer = VirtualAlloc(nullptr, sizeof(T), MEM_COMMIT, PAGE_READWRITE);
    memcpy(req->buffer, &value, sizeof(T));

    req->InUse = 1;     // Mark as processing

    
    auto start = std::chrono::steady_clock::now();
    while (InterlockedCompareExchange(&req->uready, 0, 1) != 1) {
        if (std::chrono::steady_clock::now() - start > std::chrono::seconds(1)) {
            std::cout << "[-] Timeout\n";
            return T{}; // Return default on timeout
        }
        _mm_pause();
    }

  
   
}


void exitKernel();