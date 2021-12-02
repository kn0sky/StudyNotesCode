#include "pch.h"
#include "SysMon.h"
#include "SysMonCommon.h"
#include "AutoLock.h"

#define DEVICE_NAME L"\\Device\\SysMon"
#define SYMBOL_LINK_NAME L"\\??\\SysMon"
#define DRIVER_PREFIX "SysMon: "
#define DRIVER_TAG 'lsmn'
Globals g_Globals;

EXTERN_C
NTSYSAPI
UCHAR*
PsGetProcessImageFileName(
	__in PEPROCESS Process
);

EXTERN_C
VOID
OnProcessNotify(
	_Inout_ PEPROCESS Process,
	_In_ HANDLE ProcessId,
	_Inout_opt_ PPS_CREATE_NOTIFY_INFO CreateInfo
	) {
	if (CreateInfo) {	// 进程创建时
		// 获取数据结构大小
		USHORT allocSize = sizeof(FullItem<ProcessCreateInfo>);
		// 获取命令行+数据结构大小
		USHORT CommandLineSize = 0;
		if (CreateInfo->CommandLine) {
			CommandLineSize = CreateInfo->CommandLine->Length +2 ;
			allocSize += CommandLineSize;
		}
		// 获取映像名信息加到申请内存大小里
		USHORT ImageFileNameSize = 0;
		if (CreateInfo->ImageFileName) {
			ImageFileNameSize = CreateInfo->ImageFileName->Length + 2;
			allocSize += ImageFileNameSize;
		}

		// 申请内存
		auto info = (FullItem<ProcessCreateInfo>*)ExAllocatePoolWithTag(PagedPool, allocSize, DRIVER_TAG);
		if (info == nullptr) {
			KdPrint((DRIVER_PREFIX "failed allocation!\r\n"));
			return;
		}

		// 填充固定数据
		auto& item = info->Data;
		KeQuerySystemTimePrecise(&item.Time);
		ExSystemTimeToLocalTime(&item.Time, &item.Time);
		item.Type = ItemType::ProcessCreate;
		item.Size = sizeof(FullItem<ProcessCreateInfo>) + CommandLineSize + ImageFileNameSize;
		item.ProcessId = HandleToULong(ProcessId);
		item.ParentProcessId = HandleToULong(CreateInfo->ParentProcessId);
		
		// 填充命令行相关信息
		if (CommandLineSize > 0) {
			// 如果有命令行，就复制到结构的最后，填充相关信息
			::memcpy((UCHAR*)&item + sizeof(item), CreateInfo->CommandLine->Buffer, CommandLineSize);
			item.CommandLineLength = CommandLineSize / sizeof(WCHAR);
			item.CommandLineOffset = sizeof(item);
		}
		else {
			// 如果没命令行，就赋值0
			item.CommandLineLength = 0;
			item.CommandLineOffset = 0;
		}

		// 填充映像名相关信息
		if (ImageFileNameSize > 0) {
			::memcpy((UCHAR*)&item + sizeof(item) + CommandLineSize, CreateInfo->ImageFileName->Buffer,ImageFileNameSize);
			item.ImageFileNameLength = ImageFileNameSize / sizeof(WCHAR);
			item.ImageFileNameOffset = sizeof(item) + CommandLineSize;
		}
		else {
			item.ImageFileNameLength = 0;
			item.ImageFileNameOffset = 0;
		}

		// 插入到链表中
		PushItem(&info->Entry);

	}
	else {	// 进程退出时
		// 申请空间
		auto info = (FullItem<ProcessExitInfo>*)ExAllocatePoolWithTag(PagedPool, sizeof(FullItem<ProcessExitInfo>),DRIVER_TAG);
		if (!info) {
			KdPrint((DRIVER_PREFIX "failed allocation!\r\n"));
			return;
		}
		// 数据填充
		auto& item = info->Data;
		KeQuerySystemTimePrecise(&item.Time);
		ExSystemTimeToLocalTime(&item.Time, &item.Time);
		item.Type = ItemType::ProcessExit;
		item.ProcessId = HandleToULong(ProcessId);
		item.Size = sizeof(ProcessExitInfo);

		// 插入到链表中
		PushItem(&info->Entry);
	}
}

EXTERN_C
VOID
OnThreadNotify(
	_In_ HANDLE ProcessId,
	_In_ HANDLE ThreadId,
	_In_ BOOLEAN Create
) {
	auto size = sizeof(FullItem<ThreadCreateExitInfo>);

	PEPROCESS pEprocess = NULL;
	PUCHAR ProcessImageFileName = NULL;
	auto status = PsLookupProcessByProcessId(ProcessId,&pEprocess);
	auto ProcessImageFileNameLength = 0;
	if (NT_SUCCESS(status)) {
		ProcessImageFileName = PsGetProcessImageFileName(pEprocess);
		if (ProcessImageFileName) {
			ProcessImageFileNameLength = strlen((const char*)ProcessImageFileName) +1;
			size += ProcessImageFileNameLength;
		}
	}
	

	auto info = (FullItem<ThreadCreateExitInfo>*)ExAllocatePoolWithTag(PagedPool, size, DRIVER_TAG);
	if (!info) {
		KdPrint((DRIVER_PREFIX "failed allocation!\r\n"));
		return;
	}
	auto& item = info->Data;
	KeQuerySystemTimePrecise(&item.Time);
	ExSystemTimeToLocalTime(&item.Time, &item.Time);
	item.Size = size;
	item.Type = Create ? ItemType::ThreadCreate : ItemType::ThreadExit;
	item.ProcessId = HandleToULong(ProcessId);
	item.ThreadId = HandleToULong(ThreadId);

	if (ProcessImageFileNameLength > 0) {
		::memcpy((UCHAR*)&item + sizeof(item),ProcessImageFileName,ProcessImageFileNameLength);
		item.ProcessImageFileNameLength = ProcessImageFileNameLength;
		item.ProcessImageFileNameOffset = sizeof(item);
	}
	else {
		item.ProcessImageFileNameLength = 0;
		item.ProcessImageFileNameOffset = 0;
	}

	PushItem(&info->Entry);
}

EXTERN_C
VOID
OnImageNotify(
	_In_opt_ PUNICODE_STRING FullImageName,
	_In_ HANDLE ProcessId,                // pid into which image is being mapped
	_In_ PIMAGE_INFO ImageInfo
) {
	auto size = sizeof(FullItem<ImageLoadInfo>);
	auto ImageNameSize = 0;
	if (FullImageName) {
		ImageNameSize = FullImageName->Length + 2;
		size += ImageNameSize;
	}


	auto info = (FullItem<ImageLoadInfo>*)ExAllocatePoolWithTag(PagedPool, size, DRIVER_TAG);
	if (!info) {
		KdPrint((DRIVER_PREFIX "failed allocation!\r\n"));
		return;
	}
	auto& item = info->Data;
	KeQuerySystemTimePrecise(&item.Time);
	ExSystemTimeToLocalTime(&item.Time, &item.Time);
	item.Size = size;
	item.Type = ItemType::ImageLoad;
	item.ProcessId = HandleToULong(ProcessId);
	item.ImageBase = (ULONG)(ImageInfo->ImageBase);


	if (ImageNameSize > 0) {
		::memcpy((UCHAR*)&item + sizeof(item), FullImageName->Buffer, ImageNameSize);
		item.ImageLoadPathLength = ImageNameSize;
		item.ImageLoadPathOffset = sizeof(item);
	}
	else {
		item.ImageLoadPathLength = 0;
		item.ImageLoadPathOffset = 0;
	}

	PushItem(&info->Entry);
}

EXTERN_C
NTSTATUS
SysMonRead(
	_In_ struct _DEVICE_OBJECT* DeviceObject,
	_Inout_ struct _IRP* Irp
) {
	auto stack = IoGetCurrentIrpStackLocation(Irp);
	auto len = stack->Parameters.Read.Length;
	auto status = STATUS_SUCCESS;
	auto count = 0;
	
	NT_ASSERT(Irp->MdlAddress);	// 这里使用的是直接I/O

	// 拿到缓冲区
	auto buffer = (PUCHAR)MmGetSystemAddressForMdlSafe(Irp->MdlAddress, NormalPagePriority);
	if (!buffer) {
		status = STATUS_INSUFFICIENT_RESOURCES;
	}
	else {
		AutoLock<FastMutex> lock(g_Globals.Mutex);// 要是没拿到互斥量会咋样？
		while (true) {
			// 链表空了就跳出
			if (IsListEmpty(&g_Globals.ItemsHead))
				break;
			// 从链表中拿走一项
			auto entry = RemoveHeadList(&g_Globals.ItemsHead);
			auto info = CONTAINING_RECORD(entry, FullItem<ItemHeader>, Entry);	// 为啥是这个结构
			auto size = info->Data.Size;
			if (len < size) {
				// 满了，再插进去，不拿了，跳出
				InsertHeadList(&g_Globals.ItemsHead, entry);
				break;
			}
			g_Globals.ItemCount--;
			// 把拿走的那一项复制到缓冲区
			::memcpy(buffer, &info->Data, size);
			// 计算剩余空间
			len -= size;
			// 缓冲区往后移动
			buffer += size;
			// 计算拿走的数量
			count +=size;

			ExFreePool(info);
		}		
	}
	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = count;
	IoCompleteRequest(Irp, 0);
	return status;


}

EXTERN_C NTSTATUS
ZeroCreateClose(PDEVICE_OBJECT, PIRP Irp) {
	auto status = STATUS_SUCCESS;
	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, 0);
	return status;
}

EXTERN_C
VOID
DriverUnload(
	_In_ struct _DRIVER_OBJECT* pDriverObject
) {
	PsSetCreateProcessNotifyRoutineEx(OnProcessNotify, TRUE);
	PsRemoveCreateThreadNotifyRoutine(OnThreadNotify);
	PsRemoveLoadImageNotifyRoutine(OnImageNotify);

	UNICODE_STRING symLinkName = RTL_CONSTANT_STRING(SYMBOL_LINK_NAME);
	IoDeleteSymbolicLink(&symLinkName);
	IoDeleteDevice(pDriverObject->DeviceObject);

	while (!IsListEmpty(&g_Globals.ItemsHead)) {
		auto entry = RemoveHeadList(&g_Globals.ItemsHead);
		ExFreePool(CONTAINING_RECORD(entry, FullItem<ItemHeader>, Entry));
	}
}

EXTERN_C
NTSTATUS
DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pReg) {

	// 初始化链表&初始化快速互斥体
	InitializeListHead(&g_Globals.ItemsHead);
	g_Globals.Mutex.init();

	PDEVICE_OBJECT pDeviceObject = NULL;
	UNICODE_STRING symLinkName = RTL_CONSTANT_STRING(SYMBOL_LINK_NAME);
	bool isSymbolLinkCreate = false;
	auto status = STATUS_SUCCESS;

	do {
		// 创建设备对象
		UNICODE_STRING devName = RTL_CONSTANT_STRING(DEVICE_NAME);
		status = IoCreateDevice(pDriverObject, 0, &devName, FILE_DEVICE_UNKNOWN, 0, FALSE, &pDeviceObject);
		if (!NT_SUCCESS(status)) {
			KdPrint((DRIVER_PREFIX "failed to create symbolic link (0x%08X)\n", status));
			break;
		}
		// 设置直接IO
		pDeviceObject->Flags |= DO_DIRECT_IO;

		// 创建符号链接
		status = IoCreateSymbolicLink(&symLinkName, &devName);
		if (!NT_SUCCESS(status)) {
			KdPrint((DRIVER_PREFIX "failed to create symbolic link (0x%08X)\n", status));
			break;
		}
		isSymbolLinkCreate = true;

		// 注册进程通知
		status = PsSetCreateProcessNotifyRoutineEx(OnProcessNotify, FALSE);
		if (!NT_SUCCESS(status)) {
			KdPrint((DRIVER_PREFIX "failed to register process callback (0x%08X)\n", status));
			break;
		}

		// 注册线程通知
		status = PsSetCreateThreadNotifyRoutine(OnThreadNotify);
		if (!NT_SUCCESS(status)) {
			KdPrint((DRIVER_PREFIX "failed to register thread callback (0x%08X)\n", status));
			break;
		}

		// 注册映像通知
		status = PsSetLoadImageNotifyRoutine(OnImageNotify);
		if (!NT_SUCCESS(status)) {
			KdPrint((DRIVER_PREFIX "failed to register image callback (0x%08X)\n", status));
			break;
		}
	} while (false);
	// 如果失败了,就回滚操作
	if (!NT_SUCCESS(status)) {
		if (pDeviceObject) IoDeleteDevice(pDeviceObject);
		if (isSymbolLinkCreate) IoDeleteSymbolicLink(&symLinkName);
	}

	// 设置分发例程
	pDriverObject->DriverUnload = DriverUnload;
	pDriverObject->MajorFunction[IRP_MJ_CREATE] = pDriverObject->MajorFunction[IRP_MJ_CLOSE] = ZeroCreateClose;
	pDriverObject->MajorFunction[IRP_MJ_READ] = SysMonRead;

	return status;
}


void PushItem(PLIST_ENTRY entry) {
	// 拿到互斥量,函数结束自动释放
	AutoLock<FastMutex> lock(g_Globals.Mutex);
	
	// 链表模拟队列
	// 如果数据量过大,就每增加一条就删掉一条旧数据
	if (g_Globals.ItemCount > 1024) {
		// 移除第一个成员
		auto head = RemoveHeadList(&g_Globals.ItemsHead);
		g_Globals.ItemCount--;
		// 获取该成员首地址,进行内存释放
		auto item = CONTAINING_RECORD(head, FullItem<ItemHeader>, Entry);
		ExFreePool(item);
	}
	// 插入数据到链表尾部
	InsertTailList(&g_Globals.ItemsHead, entry);
	g_Globals.ItemCount++;
}

