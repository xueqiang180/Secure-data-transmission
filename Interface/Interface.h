#pragma once
#include <iostream>
using namespace std;

//1.读共享内存
//2.加解密  ->  对称加密
class Interface
{
public:
	Interface(string json);
	~Interface();

	//加密
	string encryptData(string data);
	//解密
	string decryptData(string data);
private:
	string m_key;
};

