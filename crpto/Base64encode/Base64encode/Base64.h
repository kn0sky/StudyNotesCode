#pragma once
#include <iostream>
#include <string>

class Base64calc
{
public:
	Base64calc();
	Base64calc(char * newIndexTable,char newEntrychar = '=');
	~Base64calc();

public:
	void printIndexTable();
	char getIndex(char encodeChar);
	std::string encodeBase64(char* unencodeStr);
	std::string encodeBase64(char* unencodeStr,int test);
	std::string decodeBase64(char* encodeStr);
private:
	char indexTable[64];	// 索引表，需要64位，支持通过字符数组设定
	char entryChar;		// 空白填充符

};


struct base64unit
{
	unsigned char b1 : 2;
	unsigned char a : 6;

	unsigned char c1 : 4;
	unsigned char b2 : 4;
	
	unsigned char d : 6;
	unsigned char c2 : 2;
	
	char index1;
	char index2;
	char index3;
	char index4;

	base64unit(const void* addr, int n) {
		memset(this, 0, sizeof(base64unit));
		memcpy((void*)this, addr, n);
		index1 = a;
		index2 = b1 << 4 | b2;
		index3 = c1 << 2 | c2;
		index4 = d;
	}
};
