#pragma once
#include <ntimage.h>

void delay(int ms) {
	LARGE_INTEGER interval;
	interval.QuadPart = -ms * 1000 * 10;
	KeDelayExecutionThread(KernelMode, FALSE, &interval);
};

BOOLEAN bDataCompare(const BYTE* pData, const BYTE* bMask, const char* szMask) {
	for (; *szMask; ++szMask, ++pData, ++bMask) {
		if (*szMask == 'x' && *pData != *bMask)
			return FALSE;
	}
	return (*szMask) == 0;
}

PVOID FindSection(const char* sectionName, uintptr_t modulePtr, PULONG size) {
	size_t namelength = strlen(sectionName);
	PIMAGE_NT_HEADERS headers = (PIMAGE_NT_HEADERS)(modulePtr + ((PIMAGE_DOS_HEADER)modulePtr)->e_lfanew);
	PIMAGE_SECTION_HEADER sections = IMAGE_FIRST_SECTION(headers);
	for (DWORD i = 0; i < headers->FileHeader.NumberOfSections; ++i) {
		PIMAGE_SECTION_HEADER section = &sections[i];
		if (memcmp(section->Name, sectionName, namelength) == 0 &&
			namelength == strlen((char*)section->Name)) {
			if (!section->VirtualAddress) {
				return 0;
			}
			if (size) {
				*size = section->Misc.VirtualSize;
			}
			return (PVOID)(modulePtr + section->VirtualAddress);
		}
	}
	return 0;
}


uintptr_t FindSectionAtKernel(HANDLE device_handle, const char* sectionName, uintptr_t modulePtr, PULONG size) {
	if (!modulePtr)
		return 0;

	BYTE headers[0x1000];
	// Use RtlCopyMemory for kernel mode
	RtlCopyMemory(headers, (PVOID)modulePtr, sizeof(headers));

	ULONG sectionSize = 0;
	uintptr_t section = (uintptr_t)FindSection(sectionName, (uintptr_t)headers, &sectionSize);
	if (!section || !sectionSize) {
		DbgPrint("[-] Can't find section\n");
		return 0;
	}
	if (size)
		*size = sectionSize;

	return section - (uintptr_t)headers + modulePtr;
}

uintptr_t find_pattern(uintptr_t base, size_t range, const char* pattern, const char* mask)
{
	const auto check_mask = [](const char* base, const char* pattern, const char* mask) -> bool
		{
			for (; *mask; ++base, ++pattern, ++mask)
			{
				if (*mask == 'x' && *base != *pattern)
				{
					return false;
				}
			}

			return true;
		};

	range = range - strlen(mask);

	for (size_t i = 0; i < range; ++i)
	{
		if (check_mask((const char*)base + i, pattern, mask))
		{
			return base + i;
		}
	}

	return NULL;
}

uintptr_t find_pattern(uintptr_t base, const char* pattern, const char* mask)
{
	const PIMAGE_NT_HEADERS headers = (PIMAGE_NT_HEADERS)(base + ((PIMAGE_DOS_HEADER)base)->e_lfanew);
	const PIMAGE_SECTION_HEADER sections = IMAGE_FIRST_SECTION(headers);

	for (size_t i = 0; i < headers->FileHeader.NumberOfSections; i++)
	{
		const PIMAGE_SECTION_HEADER section = &sections[i];

		if (section->Characteristics & IMAGE_SCN_MEM_EXECUTE)
		{
			const auto match = find_pattern(base + section->VirtualAddress, section->Misc.VirtualSize, pattern, mask);

			if (match)
			{
				return match;
			}
		}
	}

	return 0;
}

extern "C" NTKERNELAPI PVOID
PsGetProcessSectionBaseAddress(
	__in PEPROCESS Process
);

PVOID GetProcessBaseAddress(HANDLE pid)
{
	PEPROCESS pProcess = NULL;
	PVOID Base = NULL;

	if (pid == 0) return NULL;

	NTSTATUS NtRet = PsLookupProcessByProcessId(pid, &pProcess);
	if (NtRet != STATUS_SUCCESS) {
		return NULL;
	}

	Base = PsGetProcessSectionBaseAddress(pProcess);

	ObDereferenceObject(pProcess);

	return Base;
}


//https://ntdiff.github.io/
#define WINDOWS_1803 17134
#define WINDOWS_1809 17763
#define WINDOWS_1903 18362
#define WINDOWS_1909 18363
#define WINDOWS_2004 19041
#define WINDOWS_20H2 19569
#define WINDOWS_21H1 20180

ULONG GetUserDirectoryTableBaseOffset()
{
	RTL_OSVERSIONINFOW ver = { 0 };
	RtlGetVersion(&ver);

	switch (ver.dwBuildNumber)
	{
	case WINDOWS_1803:
		return 0x0278;
		break;
	case WINDOWS_1809:
		return 0x0278;
		break;
	case WINDOWS_1903:
		return 0x0280;
		break;
	case WINDOWS_1909:
		return 0x0280;
		break;
	case WINDOWS_2004:
		return 0x0388;
		break;
	case WINDOWS_20H2:
		return 0x0388;
		break;
	case WINDOWS_21H1:
		return 0x0388;
		break;
	default:
		return 0x0388;
	}
}

ULONG_PTR GetProcessCr3(PEPROCESS pProcess)
{
	PUCHAR process = (PUCHAR)pProcess;
	ULONG_PTR process_dirbase = *(PULONG_PTR)(process + 0x28); //dirbase x64, 32bit is 0x18
	if (process_dirbase == 0)
	{
		ULONG UserDirOffset = GetUserDirectoryTableBaseOffset();
		ULONG_PTR process_userdirbase = *(PULONG_PTR)(process + UserDirOffset);
		return process_userdirbase;
	}
	return process_dirbase;
}


NTSTATUS ReadPhysicalAddress(LONGLONG TargetAddress, PVOID lpBuffer, SIZE_T Size, SIZE_T* BytesRead)
{
	MM_COPY_ADDRESS AddrToRead = { 0 };
	AddrToRead.PhysicalAddress.QuadPart = TargetAddress;
	return MmCopyMemory(lpBuffer, AddrToRead, Size, MM_COPY_MEMORY_PHYSICAL, BytesRead);
}

#define PAGE_OFFSET_SIZE 12
static const uint64_t PMASK = (~0xfull << 8) & 0xfffffffffull;

uint64_t TranslateLinearAddress2(uint64_t directoryTableBase, uint64_t virtualAddress) {
	// Mask to clear the last 12 bits (page offset)
	directoryTableBase &= ~0xf;

	uint64_t pageOffset = virtualAddress & ~(~0ul << PAGE_OFFSET_SIZE);
	uint64_t pte = ((virtualAddress >> 12) & (0x1ffll));
	uint64_t pt = ((virtualAddress >> 21) & (0x1ffll));
	uint64_t pd = ((virtualAddress >> 30) & (0x1ffll));
	uint64_t pdp = ((virtualAddress >> 39) & (0x1ffll));

	SIZE_T readsize = 0;
	uint64_t pdpe = 0;

	// Read the Physical Page Directory Pointer Entry (PDPTE)
	ReadPhysicalAddress(directoryTableBase + 8 * pdp, &pdpe, sizeof(pdpe), &readsize);
	if (~pdpe & 1) return 0;

	uint64_t pde = 0;
	// Read the Physical Page Directory Entry (PDE)
	ReadPhysicalAddress((pdpe & PMASK) + 8 * pd, &pde, sizeof(pde), &readsize);
	if (~pde & 1) return 0;

	// Check for 1GB large pages in PDE
	if (pde & 0x80) {
		return (pde & (~0ull << 42 >> 12)) + (virtualAddress & ~(~0ull << 30));
	}

	uint64_t pteAddr = 0;
	// Read the Physical Page Table Entry (PTE)
	ReadPhysicalAddress((pde & PMASK) + 8 * pt, &pteAddr, sizeof(pteAddr), &readsize);
	if (~pteAddr & 1) return 0;

	// Check for 2MB large pages in PTE
	if (pteAddr & 0x80) {
		return (pteAddr & PMASK) + (virtualAddress & ~(~0ull << 21));
	}

	// Read the actual Page Table Entry (PTE)
	virtualAddress = 0;
	ReadPhysicalAddress((pteAddr & PMASK) + 8 * pte, &virtualAddress, sizeof(virtualAddress), &readsize);
	virtualAddress &= PMASK;

	if (!virtualAddress) return 0;

	// Return the final physical address by adding the page offset
	return virtualAddress + pageOffset;
}
uint64_t TranslateLinearAddress(uint64_t directoryTableBase, uint64_t virtualAddress) {
	directoryTableBase &= ~0xf;

	uint64_t pageOffset = virtualAddress & ~(~0ul << PAGE_OFFSET_SIZE);
	uint64_t pte = ((virtualAddress >> 12) & (0x1ffll));
	uint64_t pt = ((virtualAddress >> 21) & (0x1ffll));
	uint64_t pd = ((virtualAddress >> 30) & (0x1ffll));
	uint64_t pdp = ((virtualAddress >> 39) & (0x1ffll));

	SIZE_T readsize = 0;
	uint64_t pdpe = 0;
	ReadPhysicalAddress(directoryTableBase + 8 * pdp, &pdpe, sizeof(pdpe), &readsize);
	if (~pdpe & 1)
		return 0;

	uint64_t pde = 0;
	ReadPhysicalAddress((pdpe & PMASK) + 8 * pd, &pde, sizeof(pde), &readsize);
	if (~pde & 1)
		return 0;

	/* 1GB large page, use pde's 12-34 bits */
	if (pde & 0x80)
		return (pde & (~0ull << 42 >> 12)) + (virtualAddress & ~(~0ull << 30));

	uint64_t pteAddr = 0;
	ReadPhysicalAddress((pde & PMASK) + 8 * pt, &pteAddr, sizeof(pteAddr), &readsize);
	if (~pteAddr & 1)
		return 0;

	/* 2MB large page */
	if (pteAddr & 0x80)
		return (pteAddr & PMASK) + (virtualAddress & ~(~0ull << 21));

	virtualAddress = 0;
	ReadPhysicalAddress((pteAddr & PMASK) + 8 * pte, &virtualAddress, sizeof(virtualAddress), &readsize);
	virtualAddress &= PMASK;

	if (!virtualAddress)
		return 0;

	return virtualAddress + pageOffset;
}
//MmMapIoSpaceEx limit is page 4096 byte
NTSTATUS WritePhysicalAddress(LONGLONG TargetAddress, PVOID lpBuffer, SIZE_T Size, SIZE_T* BytesWritten)
{
	if (!TargetAddress)
		return STATUS_UNSUCCESSFUL;

	PHYSICAL_ADDRESS AddrToWrite = { 0 };
	AddrToWrite.QuadPart = TargetAddress;

	PVOID pmapped_mem = MmMapIoSpaceEx(AddrToWrite, Size, PAGE_READWRITE);

	if (!pmapped_mem)
		return STATUS_UNSUCCESSFUL;

	memcpy(pmapped_mem, lpBuffer, Size);

	*BytesWritten = Size;
	MmUnmapIoSpace(pmapped_mem, Size);
	return STATUS_SUCCESS;
}