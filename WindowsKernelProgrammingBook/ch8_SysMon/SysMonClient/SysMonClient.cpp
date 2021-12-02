#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include "SysMonCommon.h"
#define SYMBOL_LINK_NAME L"\\\\.\\SysMon"

void DisplayTime(const LARGE_INTEGER& time) {
	SYSTEMTIME st;
	::FileTimeToSystemTime((PFILETIME)&time, &st);
	printf("%02d:%02d:%02d:%03d ",st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
}

void DisplayInfo(PBYTE buffer, DWORD size) {
	auto count = size;
	while (count >0) {
		auto header = (ItemHeader*)buffer;
		switch (header->Type) {
		case ItemType::ProcessExit: {
			DisplayTime(header->Time);
			auto info = (ProcessExitInfo*)buffer;
			printf("【Process】【Exit】Process %06d Exited\r\n",info->ProcessId);
			break;
		}
		case ItemType::ProcessCreate: {
			DisplayTime(header->Time);
			auto info = (ProcessCreateInfo*)buffer;
			printf("【Process】【Create】Process %06d Created! ImageFileName: %ws  CommandLine: %ws\n",info->ProcessId,(PWCHAR)(info->ImageFileNameOffset + buffer),(PWCHAR)(info->CommandLineOffset+buffer));
			break;
		}
		case ItemType::ThreadCreate: {
			DisplayTime(header->Time);
			auto info = (ThreadCreateExitInfo*)buffer;
			printf("【Thread】【Create】Thread: %04d Created in process %04d::%s\n",info->ThreadId,info->ProcessId,(PCHAR)(info->ProcessImageFileNameOffset + buffer));
			break;
		}
		case ItemType::ThreadExit: {
			DisplayTime(header->Time);
			auto info = (ThreadCreateExitInfo*)buffer;
			printf("【Thread】【Exit】Thread: %04d Exited from process %04d::%s\n", info->ThreadId, info->ProcessId, (PCHAR)(info->ProcessImageFileNameOffset + buffer));
			break;
		}
		case ItemType::ImageLoad: {
			DisplayTime(header->Time);
			auto info = (ImageLoadInfo*)buffer;
			printf("【ImageLoad】ProcessId: %04d ImageBase: %0x16x ImagePath: %ws\n", info->ProcessId, info->ImageBase,(PWCHAR)(info->ImageLoadPathOffset + buffer));
			break;
		}
		default:
			break;
		}
		
		buffer += header->Size;	// 向后移动指针
		count -= header->Size;	// 计算剩余大小
	}
}

int main() {
	auto hFile = ::CreateFile(SYMBOL_LINK_NAME, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		return -1;
	}

	BYTE buffer[1 << 16];
	while (true) {
		DWORD bytes;
		if (!::ReadFile(hFile, buffer, sizeof(buffer), &bytes, NULL)) {
			printf("【ERROR】ReadFile Error!!!\r\n");
			return -2;
		}

		if (bytes != 0) {
			DisplayInfo(buffer,bytes);
		}

		::Sleep(200);
	}
	return 0;
}
