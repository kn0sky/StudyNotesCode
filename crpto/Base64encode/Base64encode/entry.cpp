#include "Base64.h"
#include <iostream>
#include <string>

int main() {
	std::string str = "Hello selph!";
	std::cout << "oriStr：" << str << std::endl;
	std::cout << std::endl;

	// 标准Base64索引表编码解码
	Base64calc test;
	test.printIndexTable();
	std::string encode = test.encodeBase64((char*)str.data());
	std::string encodeEx = test.encodeBase64((char*)str.data(), 1);
	std::string decode = test.decodeBase64((char*)encode.data());
	std::cout << "encode  ：" << encode << std::endl;
	std::cout << "encodeEx：" << encodeEx << std::endl;
	std::cout << "decode  ：" << decode << std::endl;

	std::cout << std::endl;

	// 自定义Base64索引表编码解码
	char* myIndexTable =(char*) "abcdefghijklmnopqrstuvwxyz!@#$%^&*()ABCDEFGHIJKLMNOPQRSTUVWXYZ<>";
	Base64calc testCostom(myIndexTable, 'a');
	testCostom.printIndexTable();
	std::string encode1 = testCostom.encodeBase64((char*)str.data());
	std::string decode1 = testCostom.decodeBase64((char*)encode1.data());
	std::cout << "encode：" << encode1 << std::endl;
	std::cout << "decode：" << decode1 << std::endl;
	
	std::string c_encode = "c2VscGgtZGEyYmEy";
	Base64calc c_131;
	c_131.decodeBase64((char*)c_encode.data());
	std::cout << c_131.decodeBase64((char*)c_encode.data()) << std::endl;

	return 0;
}