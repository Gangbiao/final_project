#include <sys/time.h>
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <queue>

#include <ramcloud/RamCloud.h>
#include <ramcloud/Status.h>
#include <ramcloud/ClientException.h>

using namespace RAMCloud; 
using namespace std;

struct fileAttribute{
	size_t blockCount; //�ļ��ֿ���
	size_t blockSize;  //�ļ����С
	size_t fileSize;  //�ļ���С�����ֽ�Ϊ��λ
	struct timeval accessTime; //������ʱ���
	string group; //�ļ����������ڵ���
	string owner; //�ļ�������
	string checkSum; //�ļ�У���
	/*��������չ����*/
};

string keyAttrBlkCnt = "attribute.blockCount";
string keyAttrBlkSz = "attribute.blockSize";
string keyAttrFileSz = "attribute.fileSize";
string keyAttrAccTime_sec = "attribute.accessTime.tv_sec";
string keyAttrAccTime_usec = "attribute.accessTime.tv_usec";
string keyAttrGroup = "attribute.group";
string keyAttrOwner = "attribute.owner";
string keyAttrCheckSum = "attribute.checkSum";

//��������СĬ��Ϊ1M��1024*1024
const static size_t BUFSIZE = 1048576;

/* �ж�ĳ���ļ�������Ӧ�ı��Ƿ���ڣ�
 * ���ڷ���true
 * �����ڷ���false
 */
bool doesExist(const string &locator, const string &clusterName, const string &fileName)
{
	RamCloud client(locator.c_str(), clusterName.c_str()); //connect
	try{
		size_t tableId = client.getTableId(fileName.c_str());
	}catch(TableDoesntExistException){
		return false;//table does't exist
	}
	return true;//table exists
}
/*ɾ��ĳ���ļ��������Ӧ�ı�*/
int deleteFromRC(const string &locator, const string &clusterName, const string &fileName)
{
	RamCloud client(locator.c_str(), clusterName.c_str()); //locator main
	if(doesExist(locator, clusterName, fileName))
		client.dropTable(fileName.c_str()); // TODO if dos't exist'
	else
		cout<<"file(table) does't exist!!!"<<endl;
	return 0;
}

int getFromRC(const string &locator, const string &clusterName, const string &fileName)
{
	RamCloud client(locator.c_str(), clusterName.c_str()); // locator  main ������

	bool ret = doesExist(locator, clusterName, fileName);
	if(ret){
		cout<<"file does't exist!!!"<<endl;
		return 0;
	}
	//���ݱ�����ȡ���ID
	size_t tableId = client.getTableId(fileName.c_str()); //TODO if dos't exist
	cout<<"tableId is "<<tableId<<endl;

	// ��ȡ�ļ��Ŀ���
	Buffer value;
	struct fileAttribute attribute;

	client.read(tableId, keyAttrBlkCnt.c_str(), keyAttrBlkCnt.length(), &value);
	value.copy(0, sizeof(attribute.blockCount),&attribute.blockCount);
	//cout<<attribute.blockCount<<endl;	
	client.read(tableId, keyAttrBlkSz.c_str(), keyAttrBlkSz.length(), &value);
	value.copy(0, sizeof(attribute.blockSize),&attribute.blockSize);
	//cout<<attribute.blockSize<<endl;
	client.read(tableId, keyAttrFileSz.c_str(), keyAttrFileSz.length(), &value);
	value.copy(0, sizeof(attribute.fileSize),&attribute.fileSize);
	//cout<<attribute.fileSize<<endl;
	client.read(tableId, keyAttrAccTime_sec.c_str(), keyAttrAccTime_sec.length(), &value);
	value.copy(0, sizeof(attribute.accessTime.tv_sec),&attribute.accessTime.tv_sec);
	//cout<<attribute.accessTime.tv_sec<<endl;
	client.read(tableId, keyAttrAccTime_usec.c_str(), keyAttrAccTime_usec.length(), &value);
	value.copy(0, sizeof(attribute.accessTime.tv_usec),&attribute.accessTime.tv_usec);
	//cout<<attribute.accessTime.tv_usec<<endl;

	//cout<<"value.size()"<<value.size()<<endl;
	//cout<<"value.getData() return value is "<<value.copy(0, sizeof(buffer), buffer)<<endl; //TODO: ������δ���

	//����ļ���
	ofstream out;
	out.open(fileName.c_str(), ios::out|ios::binary);

	string key_head = "part";
	string tmp, key;
	char buf[BUFSIZE];
	FILE *fp;
	////fp=fopen(fileName.c_str(), "wb");
	////size_t offset=0;
	for(size_t i=1; i<=attribute.blockCount; i++)
	{
		//��size_tת��Ϊstring
		stringstream stream;
		stream<<i;
		tmp=stream.str();
		key = key_head + tmp;
		cout<<"key is "<<key<<endl;
		client.read(tableId, key.c_str(), key.length(), &value);
		value.copy(0, value.size(), buf); //�����ݿ�����buf��
		out.write(buf, value.size());
		//cout<<"offset is "<<offset<<endl;
		//cout<<"value.size() is "<<value.size()<<endl;
		//value.write(offset, value.size(), fp); ����׷��д��?
		//offset += value.size();
		//cout<<"offset is "<<offset<<endl;
	}

	out.close();
	cout<<"end of getFromRC"<<endl;
	return 0;
}

int putToRc(const string &locator, const string &clusterName, const string &fileName)
{
	RamCloud client(locator.c_str(), clusterName.c_str()); // locator  main

	bool ret = doesExist(locator, clusterName, fileName);
	if(ret){
		cout<<"file Exist already!!!"<<endl;
		return 0;
	}

	unsigned long tableId = client.createTable(fileName.c_str(), 2);
	cout<<"tableId = "<<tableId<<endl;

	char buf[BUFSIZE];
	//�����ļ�����
	ifstream in;
	in.open(fileName.c_str(), ios::in|ios::binary);
	if(!in)
	{
		cout<<"File dos't exist"<<endl;
		return -1;//�ļ�������
	}

	struct stat statBuf;
	int res = stat(fileName.c_str(), &statBuf);
	if(res != 0){
		cout<<"stat file error! errno:"<<errno<<endl;
	}

	//��ȡ�ļ�ֱ���ļ���β
	size_t  count=0;
	string key_head = "part"; // ��ֵ
	string key, tmp;

	while(!in.eof())
	{
		stringstream stream;
		count++;
		stream<<count;
		tmp=stream.str();
		cout<<count<<" "<<tmp<<endl;
		key = key_head + tmp;
		cout<<"key is "<<key<<endl;
		in.read(buf, BUFSIZE);
		size_t n = in.gcount();
		//cout<<n<<endl;
		client.write(tableId, key.c_str(), key.length(), buf, n);
		//stream.clear();
		//tmp.empty();
	}
	//���ļ�������Ϣд����Ľ�β
	cout<<"count = "<<count<<endl;

	string keyFileAttribute = "fileAttribute";
	struct fileAttribute attribute;
	attribute.blockCount = count;
	attribute.blockSize = BUFSIZE;
	attribute.fileSize = statBuf.st_size;
	struct timeval tv;
	gettimeofday(&tv, NULL);
	attribute.accessTime = tv;
	//attribute.group = "";
	//attribute.owner = "";
	//attribute.checkSum = "";

	client.write(tableId, keyAttrBlkCnt.c_str(), keyAttrBlkCnt.length(), &attribute.blockCount, sizeof(attribute.blockCount));
	client.write(tableId, keyAttrBlkSz.c_str(), keyAttrBlkSz.length(), &attribute.blockSize, sizeof(attribute.blockSize));
	client.write(tableId, keyAttrFileSz.c_str(), keyAttrFileSz.length(), &attribute.fileSize, sizeof(attribute.fileSize));
	client.write(tableId, keyAttrAccTime_sec.c_str(), keyAttrAccTime_sec.length(), &attribute.accessTime.tv_sec, sizeof(attribute.accessTime.tv_sec));
	client.write(tableId, keyAttrAccTime_usec.c_str(), keyAttrAccTime_usec.length(), &attribute.accessTime.tv_usec, sizeof(attribute.accessTime.tv_usec));
	//client.write(tableId, keyAttrGroup.c_str(), keyAttrGroup.length(), &attribute.group, sizeof(attribute.group));
	//client.write(tableId, keyAttrOwner.c_str(), keyAttrOwner.length(), &attribute.owner, sizeof(attribute.owner));
	//client.write(tableId, keyAttrCheckSum.c_str(), keyAttrCheckSum.length(), &attribute.checkSum, sizeof(attribute.checkSum));

	//�ر��ļ����룬�����
	in.close();
	return 0;
};

int main(int argc, char **argv)
{
	/*
	 *argv[1] : locator
	 *argv[2] : cmd 
	 *argv[3] : fileName
	 */

	if(argc != 4){
		cout<<"Usage "<<argv[0]<<" coordinatorLocator cmd fileName"<<endl;
		return -1;
	}
	string locator = argv[1];
	string cmd = argv[2];
	string fileName = argv[3];
	if( cmd.compare("put")==0 ){ //�ж��������ͣ�����0��ʾ���
		putToRc(locator, "main", fileName);
	}
	else if(cmd.compare("get")==0 ){
		getFromRC(locator, "main", fileName);
	}
	else if(cmd.compare("delete")==0){
		deleteFromRC(locator, "main", fileName);
	}
	//Status status;
	return 0;
}
