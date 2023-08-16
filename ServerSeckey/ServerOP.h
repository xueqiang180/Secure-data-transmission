#pragma once
#include <map>
#include "TcpServer.h"
#include "Message.pb.h"
#include "MySQLOP.h"
#include "SecKeyShm.h"
using namespace std;
// 处理客户端请求
class ServerOP
{
public:
	enum keyLen{Len16 = 16, Len24 = 24, Len32 = 32};
	ServerOP(string json);
	// 启动服务器
	void startServer();
	// 线程工作函数 -> 推荐使用
	static void* working(void* arg);
	// 友元破坏了类的封装
	friend void* workHard(void* arg);
	//秘钥协商
	string seckeyAgree(RequestMsg* resMsg);
	~ServerOP();
private:
	//生成随机数
	string getRandStr(keyLen len);

private:
	string m_serverID;  //当前服务器的ID
	string m_dbUser;
	string m_dbPwd;
	string m_dbSID;
	unsigned short m_port;
	map<pthread_t, TcpSocket*> m_list;
	TcpServer* m_server = NULL;
	SecKeyShm* m_shm;
	// 创建数据库实例对象
	MySQLOP m_sql;
};

void* workHard(void* arg);

