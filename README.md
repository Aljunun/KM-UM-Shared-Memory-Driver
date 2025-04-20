# Kernel-Mode to User-Mode Communication via MDL Mapping (No Syscalls / No Memory Sections)

## Overview

This project demonstrates efficient, low-overhead communication between Windows kernel-mode drivers and user-mode applications using **Memory Descriptor List (MDL)**-based memory mapping. Unlike conventional methods relying on system calls or memory sections, this technique provides a lightweight and direct mechanism for sharing memory.

### Key Features

- 🔒 **Secure & Controlled Mapping**  
  Memory mapping is tightly managed via MDLs, avoiding global memory sections or page file-backed objects.

- ⚡ **High Performance**  
  Eliminates the overhead of frequent syscalls, making it suitable for real-time or high-throughput applications.

- 🔄 **Full-Duplex Communication**  
  Supports both kernel-to-user and user-to-kernel data exchange.

- ❌ **No Use of `ZwCreateSection`, `NtMapViewOfSection`, or Shared Memory Objects**

---

## Architecture

```plaintext
+---------------------+      MDL Mapping       +----------------------+
|  Kernel-Mode Driver | <-------------------> |  User-Mode Application |
+---------------------+                        +----------------------+

       - Allocates non-paged pool memory
       - Builds MDL and maps to user-mode
       - Synchronization via interlocks
