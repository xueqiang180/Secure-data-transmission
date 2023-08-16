#pragma once
#include <string>
#include <mysql/mysql.h>
#include "SeckKeyNodeInfo.h"
using namespace std;



class MySQLOP
{
public:
	MySQLOP();
	~MySQLOP();

	// 连接数据库
	bool connectDB(string user, string passwd, string connstr);
	// 得到keyID -> 根据实际业务需求封装的小函数
	int getKeyID();
	bool updataKeyID(int keyID);
	bool writeSecKey(NodeSecKeyInfo* pNode);
	void closeDB();

private:
	// 获取当前时间, 并格式化为字符串
	string getCurTime();

private:
	//创建MySQL
	MYSQL mysql;
};

