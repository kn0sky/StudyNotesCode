#include <ntifs.h>

struct FastMutex {
	void init() {
		ExInitializeFastMutex(&_FastMutex);
	}
	void Lock() {
		ExAcquireFastMutex(&_FastMutex);
	}
	void UnLock() {
		ExReleaseFastMutex(&_FastMutex);
	}
private:
	FAST_MUTEX _FastMutex;
};

struct Mutex {
	void init() {
		// 必须调用该函数一次来初始化互斥量
		KeInitializeMutex(&_Mutex, 0);
	}
	void Lock() {
		// 等待互斥量
		KeWaitForSingleObject(&_Mutex, Executive, KernelMode, FALSE, nullptr);
	}
	void UnLock() {
		// 释放互斥量
		KeReleaseMutex(&_Mutex, FALSE);
	}
private:
	KMUTEX _Mutex;
};

template<typename TLock>
struct AutoLock{
	AutoLock(TLock& lock) :_lock(lock) {
		lock.Lock();
	}
	~AutoLock() {
		_lock.UnLock();
	}
private:
	TLock& _lock;
};

Mutex myMutex;
void init() {
	myMutex.init();
}
void DoWork() {
	AutoLock<Mutex> locker(myMutex);
	// 执行其他代码
}

ERESOURCE resource;
void WriteData() {
	KeEnterCriticalRegion();							// 进入关键区(屏蔽APC)
	ExAcquireResourceExclusiveLite(&resource, TRUE);	// 获取执行体资源排他锁(写)
	// ExAcquireResourceSharedLite(&resource, TRUE);	// 获取执行体资源共享锁(读)
	// ExEnterCriticalRegionAndAcquireResourceExclusive(&resource);	// 进入关键区+获取排他锁
	// ExEnterCriticalRegionAndAcquireResourceShared(&resource);	// 进入关键区+获取共享锁
	// do something
	// ExReleaseResourceAndLeaveCriticalRegion(&resource);			// 释放锁+退出关键区
	ExReleaseResourceLite(&resource);					// 释放锁
	KeLeaveCriticalRegion();	//	退出关键区
}

VOID DriverUnload(PDRIVER_OBJECT pDriverObject) {

	DbgPrint("Bye World!\n");
}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pReg) {
	pDriverObject->DriverUnload = DriverUnload;
	DbgPrint("Hello World!\n");
	return STATUS_SUCCESS;
}