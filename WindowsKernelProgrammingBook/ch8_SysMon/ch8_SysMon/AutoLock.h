#pragma once

template<typename TLock>
struct AutoLock {
	AutoLock(TLock& lock) : _lock(lock) {
		_lock.Lock();
	}

	~AutoLock() {
		_lock.UnLock();
	}

private:
	TLock& _lock;
};