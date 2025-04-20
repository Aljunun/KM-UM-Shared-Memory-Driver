# Kernel-Mode to User-Mode Communication via MDL Mapping  

## Summary

This project demonstrates  fast communication between a Windows kernel-mode driver and a user-mode application using **MDL-based memory mapping**. It achieves over **1.5 million reads per second** without relying on:

- ❌ System calls
- ❌ Memory sections
- ❌ Large non-paged pools

Instead, it leverages **MDLs**, **interlocked operations**, and a lightweight memory mapping strategy for maximum efficiency and minimal overhead.

---

## Highlights

- ⚡ **1.5M+ reads/sec** throughput
- 💡 **No use of `ZwCreateSection`, `NtMapViewOfSection`, or `CreateFileMapping`**
- 🧠 **Small, efficient memory footprint**
- 🔒 **Synchronization via `InterlockedXxx` primitives**
 
---

## How It Works

1. **Kernel Driver**  
   - Allocates a small block of non-paged memory  
   - Uses `IoAllocateMdl` + `MmBuildMdlForNonPagedPool`  
   - Maps memory to user-mode with `MmMapLockedPagesSpecifyCache`  

2. **User-Mode App**  
    - Uses interlocked flags to sync with kernel  
   - Reads data directly with no syscall overhead

---

## Synchronization

- No mutexes or events
- Uses `InterlockedCompareExchange`, `InterlockedExchange` for flagging and handshaking
- Minimal spin-waiting ensures low-latency data exchange

---

## Performance

- Benchmarked at **~1.5 million read operations per second**
- Latency remains minimal due to zero system calls and tight control over memory

 
---

## Disclaimer

This project is for educational and experimental use only.  
Direct memory mapping from kernel to user mode is powerful but must be handled with care.

---

 
