#include "globals.h"

NTSTATUS ReadRegistryADDR() {
	UNICODE_STRING RegPath;
	UNICODE_STRING AddrValueName;
	OBJECT_ATTRIBUTES ObjectAttributes;
	HANDLE KeyHandle;
	NTSTATUS Status;
	ULONG ResultLength;
	uintptr_t Buffer;

	RtlInitUnicodeString(&RegPath, L"\\Registry\\Machine\\");
	RtlInitUnicodeString(&AddrValueName, L"oAddr");

	InitializeObjectAttributes(&ObjectAttributes, &RegPath, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, NULL);

	Status = ZwOpenKey(&KeyHandle, KEY_READ, &ObjectAttributes);
	if (!NT_SUCCESS(Status)) {
		return Status;
	}

	KEY_VALUE_PARTIAL_INFORMATION KeyValueInfo;
	Status = ZwQueryValueKey(KeyHandle, &AddrValueName, KeyValuePartialInformation, &KeyValueInfo, sizeof(KeyValueInfo) + sizeof(DWORD), &ResultLength);

	if (NT_SUCCESS(Status)) {
		if (KeyValueInfo.Type == REG_QWORD && KeyValueInfo.DataLength == sizeof(uintptr_t)) {
			RtlCopyMemory(&Buffer, KeyValueInfo.Data, sizeof(uintptr_t));
			oAddr = Buffer;
		}
		else {
			Status = STATUS_INVALID_PARAMETER;
		}
	}

	ZwClose(KeyHandle);
	return Status;
}

NTSTATUS ReadRegistryPID() {
	UNICODE_STRING RegPath;
	UNICODE_STRING PidValueName;
	UNICODE_STRING AddrValueName;
	OBJECT_ATTRIBUTES ObjectAttributes;
	HANDLE KeyHandle;
	NTSTATUS Status;
	ULONG ResultLength;
	DWORD Buffer;

	RtlInitUnicodeString(&RegPath, L"\\Registry\\Machine\\");
	RtlInitUnicodeString(&PidValueName, L"oPid");

	InitializeObjectAttributes(&ObjectAttributes, &RegPath, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, NULL);

	Status = ZwOpenKey(&KeyHandle, KEY_READ, &ObjectAttributes);
	if (!NT_SUCCESS(Status)) {
		return Status;
	}

	KEY_VALUE_PARTIAL_INFORMATION KeyValueInfo;
	Status = ZwQueryValueKey(KeyHandle, &PidValueName, KeyValuePartialInformation, &KeyValueInfo, sizeof(KeyValueInfo) + sizeof(DWORD), &ResultLength);

	if (NT_SUCCESS(Status)) {
		if (KeyValueInfo.Type == REG_DWORD && KeyValueInfo.DataLength == sizeof(DWORD)) {
			RtlCopyMemory(&Buffer, KeyValueInfo.Data, sizeof(DWORD));
			oPid = Buffer;
		}
		else {
			Status = STATUS_INVALID_PARAMETER;
		}
	}

	ZwClose(KeyHandle);
	return Status;
}