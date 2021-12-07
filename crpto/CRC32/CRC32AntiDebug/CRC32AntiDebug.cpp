#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <vector>
#include <Windows.h>
#include "CRC32AntiDebug.h"

DWORD crc32Main = 0;
unsigned int crc32TableRe[256] = { 0 };
void create_crc32_tableRe() {
    for (unsigned int i = 0; i < 0x100; i++)
    {
        auto crc = i;
        auto poly = 0xEDB88320;
        for (int bit = 0; bit < 8; bit++)	// 处理 8 次
            if (crc & 1) crc = (crc >> 1) ^ poly;	// 最后一位是1，
            else			crc >>= 1;				// 最后一位是0，
        crc32TableRe[i]=crc;
    }
}

uint32_t crc32calc(void* input, int len)
{
    uint32_t crc = 0xFFFFFFFF;
    int i;
    uint8_t index;
    uint8_t* p;
    p = (uint8_t*)input;
    for (i = 0; i < len; i++)
    {
        index = (*p ^ crc);	// 取反
        crc = (crc >> 8) ^ crc32TableRe[index];	
        p++;
    }
    return crc & 0xFFFFFFFF;
}



int main(int argc)
{
    int i = 0;

    HANDLE hThread = CreateThread(NULL, NULL,(LPTHREAD_START_ROUTINE)ThreadRoutine, NULL, 0, NULL);
    while (true)
    {
        int pass = 0 ;
        printf("请输入密码:");
        scanf("%s",&pass);
        if (pass == 965741) {
            printf("Login Success!\n"); break;

        }
        else {
            printf("Login Failed!\n");
        }
    } 
    system("pause");
    return 0;
}

void ThreadRoutine() {
    create_crc32_tableRe();

    SIZE_T mainAddr = (SIZE_T)main;
    SIZE_T len = (SIZE_T)ThreadRoutine - mainAddr;
    char buf[0x100] = {0};
    ::ReadProcessMemory(GetCurrentProcess(), (LPCVOID)mainAddr, buf, len, NULL);
    crc32Main = (DWORD)crc32calc((void*)buf, len);
    
    
    while (true)
    {
        ::ReadProcessMemory(GetCurrentProcess(), (LPCVOID)mainAddr, buf, len, NULL);
        DWORD crc = (DWORD)crc32calc((void*)buf, len);
        if (crc32Main !=crc) {
            OutputDebugStringA("检测到逆向工程行为\r\n");
            printf("\r\n【ERROR】检测到逆向工程行为\r\n");
            system("pause");
            ExitProcess(-1);
        }
    }
}