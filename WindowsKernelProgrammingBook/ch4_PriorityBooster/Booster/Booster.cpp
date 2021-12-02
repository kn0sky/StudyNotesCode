#include <stdio.h>
#include <Windows.h>

#define CTL_CODE(DeviceType,Function,Method,Access)(\
((DeviceType) << 16) | ((Access)<<14) | ((Function) << 2) | Method)
#define PRIORITY_BOOSTER_DEVICE 0x8000
#define IOCTL_PRIORITY_BOOSTER_SET_PRIORITY CTL_CODE(PRIORITY_BOOSTER_DEVICE,0x800,METHOD_NEITHER,FILE_ANY_ACCESS)
typedef struct _ThreadData {
	ULONG ThreadId;		// 需要修改的线程的ID
	int Priority;		// 需要修改的优先级
}ThreadData, * PThreadData;

int Error(const char* message) {
	printf("%s (error=%d)\n",message,GetLastError());
	return 1;
}

int main(int argc,const char* argv[])
{
	if (argc < 3) {
		printf("Usage: Booster <threadid> <priority>\r\n");
		return 0;
	}

	// 打开设备对象的句柄,该函数调用会触发IRP_MJ_CREATE分发例程,如果驱动未加载,则此处会--文件未找到
	HANDLE hDevice = CreateFileW(L"\\\\.\\PriorityVooster",GENERIC_ALL,FILE_SHARE_WRITE,nullptr,OPEN_EXISTING,0,nullptr);
	// 构造缓冲区结构
	ThreadData data = { 0 };
	data.ThreadId = atoi(argv[1]);
	data.Priority = atoi(argv[2]);

	// 这里函数触发的是IRP_MJ_DEVICE_CONTROL分发例程
	DWORD ret = 0;
	BOOL success = DeviceIoControl(hDevice, IOCTL_PRIORITY_BOOSTER_SET_PRIORITY, &data, sizeof(data), nullptr, 0, &ret, nullptr);
	if (success) {
		printf("Priority change succedded!\n");
	}
	else {
		Error("Priority change failed!");
	}

	// 这里函数触发的是IRP_MJ_CLOSE分发例程
	CloseHandle(hDevice);

	return 0;
}
