#include "pch.h"

#define DEVICE_NAME L"\\Device\\Zero"
#define SYMBOL_LINK_NAME L"\\??\\Zero"
#define DRIVER_PREFIX "Zero: "


EXTERN_C NTSTATUS
CompleteIrp(PIRP pIrp, NTSTATUS status = STATUS_SUCCESS, ULONG_PTR info = 0) {
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = info;
	IoCompleteRequest(pIrp, 0);
	return status;
}

EXTERN_C NTSTATUS
ZeroRead(
	_In_ struct _DEVICE_OBJECT* DeviceObject,
	_Inout_ struct _IRP* Irp
) {
	auto stack = IoGetCurrentIrpStackLocation(Irp);
	auto len = stack->Parameters.Read.Length;
	if (len == 0)
		return CompleteIrp(Irp, STATUS_INVALID_BUFFER_SIZE);
	auto buffer = MmGetSystemAddressForMdlSafe(Irp->MdlAddress, NormalPagePriority);
	if (!buffer)
		return CompleteIrp(Irp, STATUS_INSUFFICIENT_RESOURCES);
	memset(buffer, 0, len);
	return CompleteIrp(Irp, STATUS_SUCCESS, len);
}

EXTERN_C NTSTATUS
ZeroWrite(
	_In_ struct _DEVICE_OBJECT* DeviceObject,
	_Inout_ struct _IRP* Irp
) {
	auto stack = IoGetCurrentIrpStackLocation(Irp);
	auto len = stack->Parameters.Write.Length;
	return CompleteIrp(Irp, STATUS_SUCCESS, len);
}

EXTERN_C NTSTATUS 
ZeroCreateClose(PDEVICE_OBJECT, PIRP Irp) {
	return CompleteIrp(Irp);
}

EXTERN_C VOID 
DriverUnload(PDRIVER_OBJECT DriverObject) {
	UNICODE_STRING symLink = RTL_CONSTANT_STRING(L"\\??\\Zero");
	IoDeleteSymbolicLink(&symLink);
	IoDeleteDevice(DriverObject->DeviceObject);
}

EXTERN_C NTSTATUS
DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pReg) {
	// 解引用
	UNREFERENCED_PARAMETER(pReg);
	// 设置分发例程
	pDriverObject->DriverUnload = DriverUnload;
	pDriverObject->MajorFunction[IRP_MJ_CREATE] = pDriverObject->MajorFunction[IRP_MJ_CLOSE] = ZeroCreateClose;
	pDriverObject->MajorFunction[IRP_MJ_READ] = ZeroRead;
	pDriverObject->MajorFunction[IRP_MJ_WRITE] = ZeroWrite;
	// 创建设备对象和符号链接
	UNICODE_STRING devName = RTL_CONSTANT_STRING(DEVICE_NAME);
	UNICODE_STRING symLinkName = RTL_CONSTANT_STRING(SYMBOL_LINK_NAME);
	PDEVICE_OBJECT pDeviceObject = NULL;
	auto status = STATUS_SUCCESS;

	// 使用do-while是为了出错时方便跳出代码块
	do {
		status = IoCreateDevice(pDriverObject, 0, &devName, FILE_DEVICE_UNKNOWN, 0, FALSE, &pDeviceObject);
		if (!NT_SUCCESS(status)) {
			KdPrint((DRIVER_PREFIX "failed to create symbolic link (0x%08X)\n", status));
			break;
		}
		// 设置直接IO
		pDeviceObject->Flags |= DO_DIRECT_IO;

		status = IoCreateSymbolicLink(&symLinkName, &devName);
		if (!NT_SUCCESS(status)) {
			KdPrint((DRIVER_PREFIX "failed to create symbolic link (0x%08X)\n", status));
			break;
		}
	} while (false);
	if (!NT_SUCCESS(status)) {
		if (pDeviceObject) IoDeleteDevice(pDeviceObject);
	}

	return status;
}