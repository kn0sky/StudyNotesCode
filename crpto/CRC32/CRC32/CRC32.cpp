#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstdint>
#include <vector>
/*
CRC 校验的计算：

1. 展开多项式得到 CRC 除数
2. 原数据末端加 0，多项式最高时几次就加几个
3. 从左往右，按位异或，结果位数不够长向前补 0

IEEE802.3标准CRC32多项式：x32 + x26 + x23 + x22 + x16 + x12 + x11 + x10 + x8 + x7 + x5 + x4 + x2 + x + 1
则除数为：
*/

#define DIVISOR_CRC32 0x04C11DB7
#define DIVISOR_CRC4 0x98 // 010011b

unsigned char crc4(const char* data,const int len) {
	char crc = 0;			// 余数
	int length = len;		
	
	while (length--) {		// 循环取完字符串
		crc = crc ^ *data;	// 取了8位的值
		for (int i = 0; i < 8; i++){	// 循环8次
			if (crc & 1)				// 判断最后一位是不是1
				crc = (crc >> 1) ^ 0x0c;// 往右边移动一位与0x0c = 01100b做疑惑
			else						// 最后一位不是1的话
				crc = (crc >> 1);		// 往右边移动一位
		}
		data++;				// 切换到下一个字节
	}
	return crc;
}

unsigned char crc4_h(const char* data, const int len) {
	unsigned char crc = 0;					// 有待计算的数
	int length = len;						// 字符串长度

	while (length--) {						// 循环取完字符串
		crc = crc ^ *data++;				// 取了第一个字节的值
		for (int i = 0; i < 8; i++) {		// 遍历每一个位
			if (crc & 0x80)					// 如果第一位是1
				crc = crc ^ DIVISOR_CRC4;	// 与补全到8位的多项式异或
			crc <<= 1;						// 左移一位，相当于crc后面补了1个0，8次下来会补8个0
		}
	}
	crc >>= 4;
	return crc;
}

void crc16test() {
	unsigned int crc = 'a';//计算字符a的crc16校验码
	auto poly = 0x1021;
						   //右移8位，和手动计算一样，左移相当于补0，这里相当于直接补了8个0，开始计算。
	crc <<= 8; //<<= 相当余 crc=crc<<8;
	//计算8次。
	for (int i = 0; i < 8; i++)
	{
		//如果最高位是1的话需要计算，如果不是直接左移。（左移的操作可以想象成补0)
		if ((crc & 0x8000) != 0)
		{
			crc <<= 1;
			crc = crc ^ poly;//这个说明用的是 CRC16   x16+x12+x5+1.
		}
		else
		{
			crc <<= 1;
		}
	}
	//取后16位，如果用的是crc使用的是 unsigned short 就不需要这一步了。因为进位被截断了
	crc = crc & 0xffff;
	printf("%x\r\n",crc);

	crc = 'a';
	crc <<= 8;
	poly = 0x11021;
	for (int i = 0; i < 8; i++)
	{
		if (crc & 0x8000) {
			crc <<= 1;
			crc ^= poly;
		}
		else {
			crc <<= 1;
		}
	}
	printf("%x\r\n",crc);
}



void crc8test() {
	auto input = 0x1;
	auto poly = 0x11d;//1 0001 1101:x8+x4+x3+x2+1
	for (int i = 0; i < 8; i++)
	{
		if (input & 0x80) {
			input <<= 1;
			input ^= poly;
		}
		else {
			input <<= 1;
		}
	}
	printf("%x\r\n", input);

	input = 0x1;	//0000 0001
	poly = 0x1d;	//0001 1101 
	for (int i = 0; i < 8; i++)
	{
		if (input & 0x80) {
			input <<= 1;
			input ^= poly;
			input &= 0xff;			// 是否去首位的区别在于，是否要在异或完成之后把首位去掉
		}
		else {
			input <<= 1;
		}
	}
	printf("%x\r\n", input);
}

//int crc8(const char * str) {
//	auto crc = 0;
//	auto len = strlen(str);
//	auto poly = 0x11d;//1 0001 1101:x8+x4+x3+x2+1
//	while (len--) {
//		crc^=*str++;
//		for (int i = 0; i < 8; i++)
//		{
//			if (crc & 0x80) {
//				crc <<= 1;
//				crc ^= poly;
//			}
//			else {
//				crc <<= 1;
//			}
//		}
//	}
//	return crc;
//}

unsigned char crc4(const char* str) {
	auto crc = 0;
	auto len = strlen(str);
	auto poly = 0x19 << 4;	//1 1001:x4+x3+1
	while (len--) {
		crc ^= *str++;
		for (int i = 0; i < 4; i++)
			if (crc & 0x80) crc = (crc << 1) ^ poly;
			else			crc <<= 1;
	}
	crc = (crc & 0xff) >> 4;
	return crc;
}

unsigned char crc8(const char* str) {
	auto crc = 0;
	auto len = strlen(str);
	auto poly = 0x11d;//1 0001 1101:x8+x4+x3+x2+1
	while (len--) {
		crc ^= *str++;
		for (int i = 0; i < 8; i++)
			if (crc & 0x80) crc = (crc << 1) ^ poly;
			else			crc <<= 1;
	}
	return crc;
}

unsigned short crc16(const char* str) {
	auto crc = 0;
	auto len = strlen(str);
	auto poly = 0x11021;	//1 0001 0000 0010 0001
	while (len--) {
		crc ^= *str++;
		crc <<= 8;
		for (int i = 0; i < 8; i++)
			if (crc & 0x8000) crc = (crc << 1) ^ poly;
			else			  crc <<= 1;
	}
	return crc;
}

long crc32(const char* str) {
	long crc = 0xFFFFFFFF;
	auto len = strlen(str);
	long poly = 0x104c11db7;
	while (len--) {
		crc ^= *str++;
		crc <<= 24;
		for (int i = 0; i < 8; i++)
			if (crc & 0x80000000) crc = (crc << 1) ^ poly;
			else			crc <<= 1;
	}
	crc ^= 0xFFFFFFFF;
	return crc;
}

long crc32Re(const char* str) {
	long crc = 0xFFFFFFFF;
	auto len = strlen(str);
	long poly = 0xEDB88320;
	while (len--) {
		crc = *str++;
		for (int bit = 0; bit < 8; bit++)	// 处理 8 次
			if (crc & 1) crc = (crc >> 1) ^ poly;	// 最后一位是1，
			else			crc >>= 1;				// 最后一位是0，
	}
	crc ^= 0xFFFFFFFF;
	return crc;
}

//位逆转
uint32_t bitrev(uint32_t input, int bw)
{
	int i;
	uint32_t var;
	var = 0;
	for (i = 0; i < bw; i++)
	{
		if (input & 0x01)
		{
			var |= 1 << (bw - 1 - i);
		}
		input >>= 1;
	}
	return var;
}


std::vector<unsigned int> crc32Table;
void create_crc32_table() {
	for (unsigned int i = 0; i < 0x100; i++)
	{
		auto crc = i;
		long poly = 0x104c11db7;
		crc <<= 24;
		for (int bit = 0; bit < 8; bit++)
			if (crc & 0x80000000) crc = (crc << 1) ^ poly;
			else			crc <<= 1;
		crc32Table.push_back(crc);
	}
}

std::vector<unsigned int> crc32TableRe;
void create_crc32_tableRe() {
	for (unsigned int i = 0; i < 0x100; i++)
	{
		auto crc = i;
		auto poly = bitrev(0x04c11db7, 32);	// 0xEDB88320 按位反向
		for (int bit = 0; bit < 8; bit++)	// 处理 8 次
			if (crc & 1) crc = (crc >> 1) ^ poly;	// 最后一位是1，
			else			crc >>= 1;				// 最后一位是0，
		crc32TableRe.push_back(crc);
		//table[i] = crc;
	}
}

uint32_t crc32calc(uint32_t crc, void* input, int len)
{
	int i;
	uint8_t index;
	uint8_t* p;
	p = (uint8_t*)input;
	for (i = 0; i < len; i++)
	{
		index = (*p ^ crc);	// 取反
		crc = (crc >> 8) ^ crc32TableRe[index];	//
		p++;
	}
	return crc^0xFFFFFFFF;
}

/// <summary>
///
/// </summary>
/// <repir>http://www.sunshine2k.de/coding/javascript/crc/crc_js.html</repir>
/// <returns></returns>
int main() {
	create_crc32_table();
	create_crc32_tableRe();
	//printf("%x\r\n", crc8("\x01"));
	//printf("%x\r\n", crc4("\xb3"));
	//printf("%x\r\n", crc16("\xb3"));
	printf("%x\r\n", crc32("1"));
	printf("%x\r\n", crc32Re("1"));
	printf("%x\r\n", crc32calc(0xffffffff,"12",2));


	int i = 0;
	//for each (auto var in crc32Table)
	//{
	//	printf("%08x ", var); 
	//	if (++i == 8) {
	//		printf("\n");
	//		i = 0;
	//	}
	//}
	//puts("\n");
	//i = 0;
	//for each (auto var in crc32TableRe)
	//{
	//	printf("%08x ", var);
	//	if (++i == 8) {
	//		printf("\n");
	//		i = 0;
	//	}
	//}

	return 0;
}