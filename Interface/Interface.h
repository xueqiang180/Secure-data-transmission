#pragma once
#include <iostream>
using namespace std;

//1.�������ڴ�
//2.�ӽ���  ->  �ԳƼ���
class Interface
{
public:
	Interface(string json);
	~Interface();

	//����
	string encryptData(string data);
	//����
	string decryptData(string data);
private:
	string m_key;
};

