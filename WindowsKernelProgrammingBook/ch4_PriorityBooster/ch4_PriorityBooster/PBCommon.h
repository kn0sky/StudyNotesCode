#pragma once
#include <ntifs.h>
// 参数:
// DeviceType: 设备类型,第三方驱动从0x8000开始
// Function: 用来指明操作类型,对于第三方驱动从0x800开始
// Method: 指明用户提供的输入输出缓冲区是如何传入驱动的
// Access: 指明是到达驱动程序还是来自驱动程序
#define CTL_CODE(DeviceType,Function,Method,Access)(\
((DeviceType) << 16) | ((Access)<<14) | ((Function) << 2) | Method)
#define PRIORITY_BOOSTER_DEVICE 0x8000
#define IOCTL_PRIORITY_BOOSTER_SET_PRIORITY CTL_CODE(PRIORITY_BOOSTER_DEVICE,0x800,METHOD_NEITHER,FILE_ANY_ACCESS)
typedef struct _ThreadData {
	ULONG ThreadId;		// 需要修改的线程的ID
	int Priority;		// 需要修改的优先级
}ThreadData, * PThreadData;



