#include "Base64.h"

Base64calc::Base64calc()
{
	// 生成默认索引表
	for (int i = 0; i < 64;i++ ) {
		if (i <= 25) {
			indexTable[i] = 'A' + i;
		}
		else if (i > 25 && i <= 51) {
			indexTable[i] = 'a' + i - 26;
		}
		else if(i>51 && i<=61){
			indexTable[i] = '0' + i - 52;
		}
		else {
			if (i == 62) {
				indexTable[i] = '+';
			}
			if (i == 63) {
				indexTable[i] = '/';
			}
		}
	}
	entryChar = '=';
}


Base64calc::Base64calc(char* newIndexTable, char newEntrychar)
{
	memcpy(indexTable, newIndexTable, 64);
	entryChar = newEntrychar;
}

Base64calc::~Base64calc()
{

}

void Base64calc::printIndexTable()
{
	if (indexTable != NULL) {
		printf("IndexTable: ");
		for (int i = 0; i < 64; i++)
		{
			std::cout << indexTable[i];
		}
		std::cout << std::endl;
	}
}

char Base64calc::getIndex(char encodeChar)
{
	for (char i = 0; i < 64; i++)
	{
		if (encodeChar == indexTable[i])return i;
	}
	return -1;
}


std::string Base64calc::encodeBase64(char* unencodeStr)
{
	std::string retStr;
	int strLength = strlen(unencodeStr);
	
	// 计算能完整转换的字节部分
	for (int i = 0; i < strLength - strLength % 3; i += 3) {
		char byte1 = unencodeStr[i];
		char byte2 = unencodeStr[i+1];
		char byte3 = unencodeStr[i+2];

		retStr += indexTable[byte1 >> 2];
		retStr += indexTable[byte2 >> 4 | (byte1 & 0x03) << 4];
		retStr += indexTable[byte3 >> 6 | (byte2 & 0x0F) << 2];
		retStr += indexTable[byte3 & 0x3F];
	}

	// 缺2字节时
	if (strLength % 3 == 1) {
		char byte = unencodeStr[strLength - 1];

		retStr += indexTable[byte >> 2];
		retStr += indexTable[(byte & 0x03) << 4];
		retStr += entryChar;
		retStr += entryChar;
	}

	// 缺1字节时
	if (strLength % 3 == 2) {
		char byte1 = unencodeStr[strLength - 2];
		char byte2 = unencodeStr[strLength - 1];

		retStr += indexTable[byte1 >> 2];
		retStr += indexTable[byte2 >> 4 | (byte1 & 0x03) << 4];
		retStr += indexTable[(byte2 & 0x0F) << 2];
		retStr += entryChar;
	}
	return retStr;
}

std::string Base64calc::encodeBase64(char* unencodeStr, int test)
{
	std::string retStrEx;
	int strLength = strlen(unencodeStr);

	// 计算能完整转换的字节部分
	for (int i = 0; strLength; i += 3, strLength -= 3) {
		base64unit tmp((const void*)&unencodeStr[i], strLength >= 3 ? 3 : strLength);

		retStrEx += indexTable[tmp.index1];
		retStrEx += indexTable[tmp.index2];
		if (strLength >= 3) {
			retStrEx += indexTable[tmp.index3];
			retStrEx += indexTable[tmp.index4];
		}
		else {
			retStrEx += strLength == 1 ? entryChar : indexTable[tmp.index3];
			retStrEx += entryChar;
			break;
		}
	}
	return retStrEx;
}

std::string Base64calc::decodeBase64(char* encodeStr)
{
	std::string retStr;
	int strLength = strlen(encodeStr);

	// 计算能完整转换的字节部分
	for (int i = 0; i < strLength; i += 4) {
		char byte1 = encodeStr[i];
		char byte2 = encodeStr[i + 1];
		char byte3 = encodeStr[i + 2];
		char byte4 = encodeStr[i + 3];

		retStr += (char)(getIndex(byte1) << 2) | (getIndex(byte2) >> 4);
		if (byte3 != '=')
			retStr += (getIndex(byte2) << 4) | (getIndex(byte3) >> 2);
		if (byte4 != '=')
			retStr += (getIndex(byte3) << 6) | (getIndex(byte4));
	}
	return retStr;
}

