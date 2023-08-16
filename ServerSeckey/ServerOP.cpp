#include "ServerOP.h"
#include "TcpSocket.h"
#include <unistd.h>
#include <string>
#include <fstream>
#include "RequestCodec.h"
#include "RequestFactory.h"
#include "RespondCodec.h"
#include "RespondFactory.h"
#include "RsaCrypto.h"
#include "Hash.h"
#include <json/json.h>
#include <iostream>

using namespace std;
using namespace Json;

/*
	{
		"Port":8989
	}
*/
ServerOP::ServerOP(string json)
{
	// 解析json文件, 读文件 -> Value
	ifstream ifs(json);
	Reader r;
	Value root;
	r.parse(ifs, root);
	// 将root中的键值对value值取出
	m_serverID = root["ServerID"].asString();
	m_port = root["Port"].asInt();

	// 数据库相关的信息
	m_dbUser = root["dbUser"].asString();
	m_dbPwd = root["dbPasswd"].asString();
	m_dbSID = root["dbSID"].asString();

	// 实例化一个连接mysql数据的对象
	m_sql.connectDB(m_dbUser, m_dbPwd, m_dbSID);

	// 实例化共享内存对象
	// 从配置文件中读 key/pathname
	string shmKey = root["ShmKeyPath"].asString();
	int maxNode = root["ShmMaxNode"].asInt();
	// 客户端存储的秘钥只有一个
	m_shm = new SecKeyShm(shmKey, maxNode);
}


void ServerOP::startServer()
{
	m_server = new TcpServer;
	m_server->setListen(m_port);
	while (1)
	{
		cout << "等待客户端连接...." << endl;
		TcpSocket* tcp = m_server->acceptConn();
		if (tcp == NULL)
		{
			continue;
		}
		cout << "客户端连接成功...." << endl;
		// 创建子线程
		pthread_t tid;
		// 这个回调可以是类的静态函数, 类的友元函数, 普通的函数
		// 友元是类的朋友, 但是不属于这个类
		// 友元函数可以访问当前类的私有成员
		pthread_create(&tid, NULL, workHard, this);
		m_list.insert(make_pair(tid, tcp));
	}
}

//此处   working 与 workhard  函数相同
void * ServerOP::working(void * arg)
{
	return NULL;
}

//秘钥协商
string ServerOP::seckeyAgree(RequestMsg* reqMsg)
{
	cout << "进入秘钥协商...." << endl;
	// 0. 校验签名  ->  公钥解密   ->  得到公钥
	// 将收到的公钥数据写入到磁盘文件中
	RespondInfo info;
	ofstream ofs("public.pem");
	ofs << reqMsg->data();
	ofs.close();
	//创建非对称加密对象
	RsaCrypto rsa("public.pem", false);
	//创建哈希对象
	Hash sha1(T_SHA1);
	sha1.addData(reqMsg->data());
	bool bl = rsa.rsaVerify(sha1.result(), reqMsg->sign());
	if (bl == false)
	{
		info.status = false;
		cout << "签名校验失败......." << endl;
	}
	else
	{
		// 1. 生成随机字符串(对称加密的秘钥)
		string randStr = getRandStr(Len16);
		// 2. 通过公钥加密
		string secKey = rsa.rsaPubKeyEncrypt(randStr);
		// 3. 初始化回复的数据
		info.clientID = reqMsg->clientid();
		info.serverID = m_serverID;
		info.status = true;
		info.data = secKey;
		cout << "签名校验成功......." << endl;

		// 将生成的新秘钥写入到数据库中 -> 操作 SECKEYINFO
		NodeSecKeyInfo node;
		strcpy(node.clientID, reqMsg->clientid().data());
		strcpy(node.serverID, reqMsg->serverid().data());
		strcpy(node.seckey, randStr.data());
		node.seckeyID = m_sql.getKeyID();	// 秘钥的ID
		info.seckeyID = node.seckeyID;
		node.status = 1;

		bool b1 = m_sql.writeSecKey(&node);
		if (b1)
		{
			//成功
			m_sql.updataKeyID(node.seckeyID + 1);
			//写共享内存
			m_shm->shmWrite(&node);
		}
		else 
		{
			//失败
			info.status = false;
		}
		
	}
	// 4. 序列化
	CodecFactory* factory = new RespondFactory(&info);
	Codec* c = factory->createCodec();
	//编码之后的数据
	string encMsg = c->encodeMsg();
	// 5. 返回数据
	return encMsg;
}

ServerOP::~ServerOP()
{
	if (m_server)
	{
		delete m_server;
	}
	delete m_shm;
}

//回调函数
void* workHard(void * arg)
{
	//休眠一段时间，防止子线程与主线程不同步
	sleep(1);
	//通过参数将传递的this对象转换
	ServerOP* op = (ServerOP*)arg;
	//从op中将通信的套接字对象取出   pthread_self()表示当前的线程ID
	TcpSocket* tcp = op->m_list[pthread_self()];
	// 1. 接收客户端数据 -> 编码之后的
	string msg = tcp->recvMsg();
	// 2. 反序列化 -> 得到原始数据 RequestMsg 类型
	CodecFactory* fac = new RequestFactory(msg);
	Codec* c = fac->createCodec();
	RequestMsg* req = (RequestMsg*)c->decodeMsg();

	// 3. 取出数据
	// 判断客户端是什么请求
	string data = string();
	switch (req->cmdtype())
	{
	case 1:
		// 秘钥协商
		data = op->seckeyAgree(req);
		break;
	case 2:
		// 秘钥校验
		break;
	case 3:
		// 秘钥注销
		break;
	default:
		break;
	}

	//释放资源
	delete fac;
	delete c;

	//发送数据,关闭连接
	tcp->sendMsg(data);
	tcp->disConnect();
	//从列表中删除
	op->m_list.erase(pthread_self());
	delete tcp;

	return NULL;
}

//生成随机字符串
string ServerOP::getRandStr(keyLen len)
{
	srand(time(NULL));	// 以当前时间为种子
	string retStr = string();
	char* buf = "~`@#$%^&*()_+=-{}[];':";
	for (int i = 0; i < len; ++i)
	{
		int flag = rand() % 4;   //4种情况
		switch (flag)
		{
		case 0:	// 0-9
			retStr.append(1, rand() % 10 + '0');
			break;
		case 1:	// a-z
			retStr.append(1, rand() % 26 + 'a');
			break;
		case 2:	// A-Z
			retStr.append(1, rand() % 26 + 'A');
			break;
		case 3:	// 特殊字符
			retStr.append(1, buf[rand() % strlen(buf)]);
			break;
		}
	}
	return retStr;
}
