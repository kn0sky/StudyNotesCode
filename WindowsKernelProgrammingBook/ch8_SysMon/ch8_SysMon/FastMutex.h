#pragma once
struct FastMutex {
	void init(); 
	void Lock(); 
	void UnLock();
private:
	FAST_MUTEX _FastMutex;
};