#include <ntddk.h>

VOID DriverUnload(PDRIVER_OBJECT pDriver) {
	UNREFERENCED_PARAMETER(pDriver);
	DbgPrint("Bye World!\n");
}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pReg) {
	UNREFERENCED_PARAMETER(pReg);
	pDriverObject->DriverUnload = DriverUnload;
	DbgPrint("Hello World!\n");
	return STATUS_SUCCESS;
}