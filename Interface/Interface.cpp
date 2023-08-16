#include "Interface.h"
#include "AesCrypto.h"
#include <fstream>
#include <json/json.h>
#include "SecKeyShm.h"

using namespace Json;

Interface::Interface(string json)
{
	// 读json文件
	ifstream ifs(json);
	//得到打开共享内存的key
	Reader rd;
	Value root;
	rd.parse(ifs, root);
	
	string sid = root["ServerID"].asString();
	string cid = root["ClientID"].asString();
	string shmkey = root["ShmKeyPath"].asString();
	int maxNode = root["ShmMaxNode"].asInt();

	//查找对应的秘钥
	SecKeyShm shm(shmkey, maxNode);
	NodeSecKeyInfo node = shm.shmRead(cid, sid);
	m_key = string(node.seckey);
}

Interface::~Interface()
{
}

//加密
string Interface::encryptData(string data)
{
	//构造对象需要秘钥
	AesCrypto aes(m_key);
	string str = aes.aesCBCEncrypt(data);
	return str;
}

//解密
string Interface::decryptData(string data)
{
	//构造对象需要秘钥
	AesCrypto aes(m_key);
	string str = aes.aesCBCDecrypt(data);
	return str;
}
