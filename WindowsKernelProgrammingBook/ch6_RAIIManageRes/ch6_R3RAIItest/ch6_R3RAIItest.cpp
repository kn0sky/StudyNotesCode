#include <stdio.h>
#include <Windows.h>

template<typename T = void>
class kunique_ptr
{
public:
	kunique_ptr(T* p = nullptr) :_p(p) {}
	~kunique_ptr() {
		if (_p)VirtualFree(_p);
	}
	T* operator->()const {
		return _p;
	}
	T& operator*()const {
		return *_p;
	}
private:
	T* _p;
};

int main()
{
	kunique_ptr<void> a((void*)VirtualAlloc(NULL, 0x10, MEM_COMMIT, PAGE_READWRITE));
	memcpy(&a, "hello world",sizeof("hello world"));
	return 0;
}