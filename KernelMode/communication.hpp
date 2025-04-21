#pragma once
#include "driver.h"
#include "Ustruct.h"
#include "registry.hpp"
#include "memory.hpp"
 
  

OBJECT_ATTRIBUTES objAttr; 
OBJECT_ATTRIBUTES objAttr2; 
HANDLE hakevent;
PMDL mdl = NULL;
PVOID kernelMappedAddress = NULL;
PEPROCESS targetProcess = NULL;

PreqEntry req;
 
namespace usermode {
    NTSTATUS MdlBoss(
        PVOID UserBuffer,
        SIZE_T Size,
        PEPROCESS Process,
        PMDL* OutMdl,
        PVOID* KernelMappedAddress
    ) {
        PMDL mdl = NULL;
        PVOID kernelAddress = NULL;
        ULONG_PTR processCr3 = GetProcessCr3(Process);
        ULONG_PTR userVa = (ULONG_PTR)UserBuffer;
        ULONG pageCount = ADDRESS_AND_SIZE_TO_SPAN_PAGES(userVa, Size);

        if (!UserBuffer || !Size || !OutMdl || !KernelMappedAddress || !Process) {
            return STATUS_INVALID_PARAMETER;
        }
		printfx("[+] MdlBoss: UserBuffer: %p, Size: %zu, Process: %p\n", UserBuffer, Size, Process);
        // Allocate MDL with space for the PFN array
        mdl = IoAllocateMdl(NULL, Size, FALSE, FALSE, NULL);
        if (!mdl) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }
		printfx("[+] MdlBoss: MDL allocated: %p\n", mdl);
        // Manually populate MDL fields
        mdl->StartVa = (PVOID)(userVa & ~(PAGE_SIZE - 1)); // Page-aligned VA
        mdl->ByteOffset = userVa & (PAGE_SIZE - 1);         // Offset within the first page
        mdl->ByteCount = Size;                               // Total buffer size
        mdl->MdlFlags |= MDL_PAGES_LOCKED;                   // Pretend pages are locked (RISKY)

		printfx("[+] MdlBoss: MDL fields populated\n");
        // Get the PFN array from the MDL
        PPFN_NUMBER pfns = (PPFN_NUMBER)(mdl + 1);

        // Translate each page's VA to PA using the target process's CR3
		printfx("[+] MdlBoss: Translating VA to PA\n");
        for (ULONG i = 0; i < pageCount; i++) {
            ULONG_PTR currentVa = userVa + (i * PAGE_SIZE);
            ULONG_PTR pa = TranslateLinearAddress2(processCr3, currentVa);

            if (!pa) {
                IoFreeMdl(mdl);
                return STATUS_UNSUCCESSFUL;
            }

            pfns[i] = (PFN_NUMBER)(pa >> PAGE_SHIFT); // Store PFN in the MDL
        }
       
        // Map the MDL to kernel address space
		printfx("[+] MdlBoss: Mapping MDL to kernel address space\n");
        __try {
            kernelAddress = MmMapLockedPagesSpecifyCache(
                mdl,
                KernelMode,
                MmNonCached,
                NULL,
                FALSE,
                NormalPagePriority
            );
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
			printfx("[-] Exception while mapping MDL: %x\n", GetExceptionCode());
            IoFreeMdl(mdl);
            return GetExceptionCode();
        }

        if (!kernelAddress) {
			printfx("[-] Failed to map MDL to kernel address space\n");
            IoFreeMdl(mdl);
            return STATUS_INSUFFICIENT_RESOURCES;
        }
		printfx("[+] MdlBoss: MDL mapped to kernel address space: %p\n", kernelAddress);

        *OutMdl = mdl;
        *KernelMappedAddress = kernelAddress;
        return STATUS_SUCCESS;
    }

    NTSTATUS updateAddr() {
    
        NTSTATUS status = PsLookupProcessByProcessId((HANDLE)oPid, &targetProcess);

        if (!NT_SUCCESS(status)) {
            return status;
        }

        // Map user buffer using physical addresses from target process's CR3
        status = MdlBoss((PVOID)oAddr, sizeof(holyque), targetProcess, &mdl, &kernelMappedAddress);

        if (NT_SUCCESS(status)) {
            __try {
                req = (PreqEntry)kernelMappedAddress;





            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                status = GetExceptionCode();
            }

           
        }
        ObDereferenceObject(targetProcess);
        

        return status;
    }


  
    template <typename T>
    T read(uintptr_t address) {
        SIZE_T WByte;
        T value;

        NTSTATUS status = ReadProcessMemory(
            reinterpret_cast<HANDLE>(oPid),
            reinterpret_cast<PVOID>(address),
            &value,
            sizeof(T),
            &WByte
        );
        if (!NT_SUCCESS(status)) { return NULL; }

        return value;
    }

  

 
  


    // Connection initlisation with usermode
    NTSTATUS InitializeCommunication() {
        NTSTATUS status = ReadRegistryADDR();
        if (!NT_SUCCESS(status)) {
            DbgPrint("Failed to read Addr from registry\n");
            return status;
        }

        status = ReadRegistryPID();
        if (!NT_SUCCESS(status)) {
            DbgPrint("Failed to read PID from registry\n");
            return status;
        }
      
           
       

        status = usermode::updateAddr();
		if (!NT_SUCCESS(status)) {
			DbgPrint("Failed to update Addr\n");
			return status;
		}
		printfx("[+] successfully Mapped\n");

        printfx("[+] req: %d\n", req->Op);
         
        return status;
    }
    void exit() {

         printfx("Goodbye ! \n");
  
         MmUnmapLockedPages(kernelMappedAddress, mdl);
         IoFreeMdl(mdl);

        
    }
}

namespace game {
    NTSTATUS read() {
        NTSTATUS status = STATUS_SUCCESS;

 
        
        SIZE_T WByte;
        PVOID value = 0;

          status = ReadProcessMemory(
            reinterpret_cast<HANDLE>(req->g_pid),
            reinterpret_cast<PVOID>(req->TargetAddress),
            &value,
            req->Size,
            &WByte
        );
		req->buffer = value;

         InterlockedExchange(&req->uready, 1);

        return status;
    }

    NTSTATUS write() {
        SIZE_T WByte;
        PVOID value = reinterpret_cast<PVOID>(comm.value);

        NTSTATUS status = WriteProcessMemory(
            reinterpret_cast<HANDLE>(req->g_pid),
            reinterpret_cast<PVOID>(req->TargetAddress),
            &value,
            req->Size,
            &WByte
        );
        if (!NT_SUCCESS(status)) { return status; }
 
        InterlockedExchange(&req->uready, 1);

        return status;
    };
}
