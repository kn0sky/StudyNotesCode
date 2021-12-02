#pragma once
#include "FastMutex.h"

template<typename T>
struct FullItem {
	LIST_ENTRY Entry;
	T Data;
};

// 保存驱动全部状态
struct Globals {
	LIST_ENTRY ItemsHead;	// 驱动链表头部
	int ItemCount;			// 项数
	FastMutex Mutex;		// 快速互斥体
};

EXTERN_C void PushItem(PLIST_ENTRY entry);
