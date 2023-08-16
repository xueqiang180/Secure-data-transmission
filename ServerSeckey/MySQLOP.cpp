#include "MySQLOP.h"
#include <iostream>
#include <string.h>
#include <time.h>
using namespace std;

MySQLOP::MySQLOP()
{
	// 初始化句柄
	mysql_init(&mysql);
}


MySQLOP::~MySQLOP()
{
	//mysql_close(&mysql);
}

bool MySQLOP::connectDB(string user, string passwd, string SID)
{
	
	// 连接的数据库（句柄、主机名、用户名、密码、数据库名、端口号、socket指针、标记）
	if (!mysql_real_connect(&mysql, SID.c_str(), user.c_str(), passwd.c_str(), "ssl_test", 3306, 0, 0))
	{
		cout << "数据库连接失败" << mysql_error(&mysql) << endl;
		return false;
	}
	cout << "数据库连接成功..." << endl;
	return true;
}

int MySQLOP::getKeyID()
{
	// 查询数据库
	// for update: 临时对数据表加锁
	string sql = "select ikeysn from keyID;";
	if (mysql_query(&mysql, sql.c_str()) != 0)
	{
		//mysql_errno 返回错误编号  mysql_error 返回错误信息
		cout << "数据查询失败：" << mysql_error(&mysql) << endl;
	}
	// 该表只有一条记录
	int keyID = -1;
	//cout << "数据查询成功" << endl << endl;

    // 创建存放结果的结构体
	MYSQL_ROW row;
	// 创建数据库回应结构体
	MYSQL_RES* res = nullptr;
	// 装载结果集
	res = mysql_store_result(&mysql);
	int fields = mysql_num_fields(res);

	if (res == nullptr)
	{
		cout << "装载数据失败: " << mysql_errno(&mysql) << endl;
	}
	else
	{
		///< 取出结果集中内容
		while (row = mysql_fetch_row(res))
		{
			//for (int i = 0; i < fields; i++) {
			//	cout << row[i] << "  ";
			//}
			//cout << endl;
			keyID = atoi(row[fields - 1]);
			//标准库函数 auio() char* to int
		}
	}
	//释放结果集
	mysql_free_result(res);
	return keyID;
}

// 秘钥ID在插入的时候回自动更新, 也可以手动更新
bool MySQLOP::updataKeyID(int keyID)
{
	// 更新数据库
	string sql = "update keyID set ikeysn=" + to_string(keyID) + ";";
	//string sql = "insert into keyID(ikeysn) values(19);";
	//+ " " +"where ikeysn=" + to_string(ikeysn)
	cout << "update sql :" << sql << endl;

	if (mysql_autocommit(&mysql, 1))
	{   
		cout << "事务开启失败：" << mysql_error(&mysql) << endl;
		return false;
	}
	else
	{
		if (mysql_query(&mysql, sql.c_str()) != 0)
		{
			cout << "数据插入失败：" << mysql_error(&mysql) << endl;
			return false;
		}
		else
		{
		    cout << "数据更新成功！" << endl;
			return true;
		}
	}
}



// 将生成的秘钥写入数据库
// 更新秘钥编号
bool MySQLOP::writeSecKey(NodeSecKeyInfo* pNode)
{
	// 组织待插入的sql语句
	char sql[1024] = { 0 };
	sprintf(sql, "Insert Into seckeyinfo(clientid, serverid, keyid, createtime, state, seckey) \
					values ('%s', '%s', %d, NOW() , %d, '%s'); ", 
		pNode->clientID, pNode->serverID, pNode->seckeyID, 1, pNode->seckey);

	cout << "insert sql: " << sql << endl;

	//开启事务
	if (mysql_autocommit(&mysql, 1))
	{   /*
		   mysql 中事务默认自动提交，0设置为手动，1设置为自动
		   返回值：0表示事务开启成功，1表示失败
		*/
		cout << "事务开启失败：" << mysql_error(&mysql) << endl;
		return false;
	}
	else
	{
		if (mysql_query(&mysql, sql) != 0)
		{
			cout << "数据插入失败：" << mysql_error(&mysql) << endl;
			//mysql_errno 返回错误编号
			//mysql_error 返回错误信息
			return false;
		}
		else
		{
			//int flag = 0;
			//cout << "提交 0 | 回滚 1" << endl;
			//cin >> flag;
			//switch (flag)
			//{
			//case 0:
			//	if (mysql_commit(&mysql) != 0)
			//	{
			//		cout << "事务提交失败：" << mysql_error(&mysql) << endl;
			//		return false;
			//	 }
			//	else
			//	{
			//		cout << "密钥信息插入成功！" << endl;
			//	 }
			//	break;
			//case 1:
			//	if (mysql_rollback(&mysql) != 0)
			//	{
			//		cout << "事务回滚失败：" << mysql_error(&mysql) << endl;
			//		return false;
			//	 }
			//	else
			//	{
			//		cout << "事务回滚成功！" << endl;
			//	 }
			//	break;
			//default:
			//	break;
			//}
		}
	}
	return true;
}

void MySQLOP::closeDB()
{   //关闭MySQL
	mysql_close(&mysql);
	cout << "与数据库主动断开连接！" << endl;
}

//返回当前时间的字符串格式
string MySQLOP::getCurTime()
{
	time_t timep;
	time(&timep);
	char tmp[64];
	strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", localtime(&timep));
	//strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M", localtime(&timep));
	return tmp;
}
