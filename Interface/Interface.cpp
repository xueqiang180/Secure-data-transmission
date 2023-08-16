#include "Interface.h"
#include "AesCrypto.h"
#include <fstream>
#include <json/json.h>
#include "SecKeyShm.h"

using namespace Json;

Interface::Interface(string json)
{
	// ��json�ļ�
	ifstream ifs(json);
	//�õ��򿪹����ڴ��key
	Reader rd;
	Value root;
	rd.parse(ifs, root);
	
	string sid = root["ServerID"].asString();
	string cid = root["ClientID"].asString();
	string shmkey = root["ShmKeyPath"].asString();
	int maxNode = root["ShmMaxNode"].asInt();

	//���Ҷ�Ӧ����Կ
	SecKeyShm shm(shmkey, maxNode);
	NodeSecKeyInfo node = shm.shmRead(cid, sid);
	m_key = string(node.seckey);
}

Interface::~Interface()
{
}

//����
string Interface::encryptData(string data)
{
	//���������Ҫ��Կ
	AesCrypto aes(m_key);
	string str = aes.aesCBCEncrypt(data);
	return str;
}

//����
string Interface::decryptData(string data)
{
	//���������Ҫ��Կ
	AesCrypto aes(m_key);
	string str = aes.aesCBCDecrypt(data);
	return str;
}
