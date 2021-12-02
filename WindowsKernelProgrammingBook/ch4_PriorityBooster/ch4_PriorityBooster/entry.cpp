#include <ntifs.h>
#include <ntddk.h>
#include "PBCommon.h"
#define DEVICE_NAME L"\\Device\\PriorityBooster"
#define SYMBLE_LINK_NAME L"\\??\\PriorityVooster"

_Use_decl_annotations_
NTSTATUS
PriorityBoosterCreateClose(
	_In_ struct _DEVICE_OBJECT* DeviceObject,
	_Inout_ struct _IRP* Irp
) {
	Irp->IoStatus.Status = STATUS_SUCCESS;	// 指用什么状态完成该请求
	Irp->IoStatus.Information = 0;			// 不同请求有不同含义,这里请求需要设置为0
	IoCompleteRequest(Irp,IO_NO_INCREMENT);	// 完成IRP响应,会把IRP传回给创建者(IO管理器),然后管理器通知客户程序操作完成情况
	return STATUS_SUCCESS;
}

_Use_decl_annotations_
NTSTATUS
PriorityBoosterDeviceControl(
	_In_ struct _DEVICE_OBJECT* DeviceObject,
	_Inout_ struct _IRP* Irp
) {
	auto stack = IoGetCurrentIrpStackLocation(Irp);
	auto status = STATUS_SUCCESS;

	switch (stack->Parameters.DeviceIoControl.IoControlCode) {
	case IOCTL_PRIORITY_BOOSTER_SET_PRIORITY: {
		// do the work
		// 判断接收到的缓冲区大小是否足够
		if (stack->Parameters.DeviceIoControl.InputBufferLength < sizeof(ThreadData)) {
			status = STATUS_BUFFER_TOO_SMALL;
			break;
		}
		// 获取缓冲区参数
		auto data = (PThreadData)stack->Parameters.DeviceIoControl.Type3InputBuffer;
		if (data == nullptr) {
			status = STATUS_INVALID_PARAMETER;
			break;
		}
		__try {
			// 参数合法性检查
			if (data->Priority < 1 || data->Priority>31) {
				status = STATUS_INVALID_PARAMETER;
				break;
			}
			// 将线程id转换成句柄,然后通过API获取线程结构,该API会增加线程结构的引用计数,需要手动减少
			PETHREAD pEthread;
			status = PsLookupThreadByThreadId(ULongToHandle(data->ThreadId), &pEthread);
			if (!NT_SUCCESS(status)) {
				break;
			}
			// 修改优先级
			KeSetPriorityThread(pEthread, data->Priority);

			// 手动减少引用计数
			ObDereferenceObject(pEthread);
		}
		__except(EXCEPTION_EXECUTE_HANDLER){	// EXCEPTION_EXECUTE_HANDLER表示任何异常都会处理
			status = STATUS_ACCESS_VIOLATION;
		}
		break;

	}
	default: {
		status = STATUS_INVALID_DEVICE_REQUEST;
		break; 
	}
	}
	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;

}

VOID DriverUnload(PDRIVER_OBJECT pDriverObject) {
	// 还原驱动所做的操作
	// 删除符号链接
	UNICODE_STRING symLink = RTL_CONSTANT_STRING(SYMBLE_LINK_NAME);
	IoDeleteSymbolicLink(&symLink);
	IoDeleteDevice(pDriverObject->DeviceObject);
	KdPrint(("Bye World!\n"));
}

EXTERN_C NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pReg) {
	// 指定Unload例程
	pDriverObject->DriverUnload = DriverUnload;
	// 操作设备对象至少需要CREATE和CLOSE操作分发例程
	pDriverObject->MajorFunction[IRP_MJ_CREATE] = PriorityBoosterCreateClose;
	pDriverObject->MajorFunction[IRP_MJ_CLOSE] = PriorityBoosterCreateClose;
	// 通过设备对象使得信息能从R3传入驱动,需要使用DeviceIoControl,该API通知的是DEVICE_CONTROL
	// 通过这种方法进行通信需要控制码和输入缓冲区
	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = PriorityBoosterDeviceControl;

	// 创建设备对象,经典的驱动程序只需要一个设备对象,通过符号链接指向它即可
	UNICODE_STRING devName = RTL_CONSTANT_STRING(DEVICE_NAME);
	PDEVICE_OBJECT pDeviceObject = NULL;
	NTSTATUS ntStatus = IoCreateDevice(pDriverObject, 0, &devName, FILE_DEVICE_UNKNOWN, 0, FALSE, &pDeviceObject);

	// 提供符号链接
	UNICODE_STRING symLink = RTL_CONSTANT_STRING(SYMBLE_LINK_NAME);
	ntStatus = IoCreateSymbolicLink(&symLink, &devName);
	if (!NT_SUCCESS(ntStatus)) {
		// 如果失败了,需要回退之前的操作,因为DriverEntry中返回了非成功的状态,DriverUnload是不会被调用执行的
		KdPrint(("Failed to create symbolic link (0x%08x)\n",ntStatus));
		IoDeleteDevice(pDeviceObject);
		return ntStatus;
	}


	KdPrint(("Hello World!\n"));
	return STATUS_SUCCESS;
}