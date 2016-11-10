#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

#include <ramcloud/RamCloud.h>
#include <ramcloud/PerfStats.h>
#include <ramcloud/WireFormat.h>
using namespace RAMCloud; 
using namespace std;

//缓冲区大小默认为1M，1024*1024=1048576
const static unsigned long BUFSIZE = 1048576;

/*对从serverControlAll的返回的rawData进行解析，将解析结果保存到vetcor<PerfStats>中
 *输入：Buffer *rawData
 *    集群中服务器节点的信息包
 *输出：vector<PerfStats>
 *    包含各个服务器节点的PerfStats的vector
 *返回类型：void
 */
/**
 * Given the raw response returned by CoordinatorClient::serverControlAll,
 * divide it up into individual PerfStats objects for each server, and
 * store those in an array indexed by server id.
 *
 * \param rawData
 *		Response buffer from a call to CoordinatorClient::serverControlAll.
 * \param[out] results
 *		Filled in (possibly sparsely) with contents parsed from rawData.
 *		Entry i will contain PerfStats for the server whose ServerId has
 *		indexNumber i. Empty entries have 0 collectionTimes.
 */
void parseStats(Buffer *rawData, vector<PerfStats> *results)
{
	results->clear();
	uint32_t offset = sizeof(WireFormat::ServerControlAll::Response);
	while (offset < rawData->size()) {
		WireFormat::ServerControl::Response* header = rawData->getOffset<WireFormat::ServerControl::Response>(offset);
		offset += sizeof32(*header);
		if ((header == NULL) ||((offset + sizeof32(PerfStats)) > rawData->size())) {
			break;
		}
		uint32_t i = ServerId(header->serverId).indexNumber();
		if (i >= results->size()) {
			results->resize(i+1);
		}
		rawData->copy(offset, header->outputLength, &results->at(i));
		offset += header->outputLength;
	}
}


/*for debug*/
/*uint64_t	logUsedBytes
  Total space used (for both live and garbage collectable data) in log (the number of seglets currently used * size of seglet
  uint64_t	logFreeBytes
  Unused log space ready for write even before log cleaning (the number of free seglets * size of seglet) 
  uint64_t	logLiveBytes
  Total log space occupied to store live data that is not currently cleanable. 
  uint64_t	logMaxLiveBytes
  The largest value the logLiveBytes allowed to be. 
  uint64_t	logAppendableBytes
  Log space available to write new data (logMaxLiveBytes - logLiveBytes) 
  uint64_t	logUsedBytesInBackups*/
void myPrint(const PerfStats &stats){
	cout<<"----------------------------------------"<<endl;
	cout<<"logSizeBytes is (Mb)" <<stats.logSizeBytes/1048576<<endl;
	cout<<"logUsedBytes is(Mb) "<<stats.logUsedBytes/1048576<<endl;
	cout<<"logFreeByte is(Mb) "<<stats.logFreeBytes/1048576<<endl;
	cout<<"logLiveByte is(Mb) "<<stats.logLiveBytes/1048576<<endl;
	cout<<"logMaxLiveBytes is(Mb)"<<stats.logMaxLiveBytes/1048576<<endl;
	cout<<"logAppendableBytes is(Mb)"<<stats.logAppendableBytes/1048576<<endl;
	cout<<"logUsedBytesInBackups is(Mb)"<<stats.logUsedBytesInBackups/1048576<<endl;
}
/*获取整个集群中服务器节点的内存使用信息
 *
 *
 */
void getAllServerStats(RamCloud *client, vector<PerfStats> *vecOfPerfStats)
{
	Buffer output;
	client->serverControlAll(WireFormat::GET_PERF_STATS, "abc", 3, &output);
	parseStats(&output, vecOfPerfStats);//解析后存到vector中
	vector<PerfStats>::iterator it;
	cout<<"vecPerfStats.size() is "<<vecOfPerfStats->size()<<endl;
	for(it=vecOfPerfStats->begin()+1; it != vecOfPerfStats->end(); it++)
		myPrint(*it);   //遍历vector并打印出每个server节点的信息
}


int main(int argc, char **argv)
{
	/*
	 *locator = argv[1]
	 *cmd = argv[2]
	 *fileName = argv[3]
	 **/
	if(argc != 4){
		cout<<"Usage "<<argv[0]<<" coordinatorLocator cmd fileName"<<endl;
		return -1;
	}
	Context *context;
	string locator = argv[1];
	RamCloud client(locator.c_str(), "main");
	unsigned long tableId = client.createTable("testTable", 2);

	Buffer statsBuffer;
	PerfStats perfStats;
	client.objectServerControl(tableId, "abc", 3,
			WireFormat::ControlOp::GET_PERF_STATS, NULL, 0,
			&statsBuffer);
	perfStats = *statsBuffer.getStart<PerfStats>();
	myPrint(perfStats);
		//try{
		//client.dropTable("hello");
		//	}catch(TableDoesntExistException)
		//{
		//	cout<<"biao bu cun zai"<<endl;
		//}
	vector<PerfStats> vecOfPerfStats;
	getAllServerStats(&client, &vecOfPerfStats);
	Buffer output;
	client.serverControlAll(WireFormat::GET_PERF_STATS, "abc", 3, &output);
	//vector<PerfStats> vecPerfStats;
	//parseStats(&output, &vecPerfStats);
	//vector<PerfStats>::iterator it;
	//cout<<"vecPerfStats.size() is "<<vecPerfStats.size()<<endl;
	//for(it=vecPerfStats.begin(); it != vecPerfStats.end(); it++)
	//	myPrint(*it);

	//size_t Id[101] = {0};
	//for(int i=1; i<=10; i++)
	//{
	//	stringstream sstream;
	//	sstream<<i;
	//	string tableName = "table" + sstream.str();
	//Id[i] = client.createTable(tableName.c_str(), 2);
	//}
	//size_t Id1 = client.createTable("table1");
	//size_t Id2 = client.createTable("table2");
	//size_t Id3 = client.createTable("table3");

	//ServerMetrics server_metrics;
	//server_metrics=client.getMetrics(locator.c_str());
	//cout<<"server_metrics.size()="<<server_metrics.size()<<endl;
	//cout<<*client.getServiceLocator()<<endl;

	//Buffer statsBuffer;
	//PerfStats stats;
	//client.objectServerControl(Id[1], "abc", 3,	WireFormat::ControlOp::GET_PERF_STATS, NULL, 0, &statsBuffer);
	//cout<<"statsBuffer.size()"<<statsBuffer.size()<<endl;
	//stats = *statsBuffer.getStart<PerfStats>();	
	//myPrint(stats);

	//client.objectServerControl(Id[2], "abc", 3,	WireFormat::ControlOp::GET_PERF_STATS, NULL, 0, &statsBuffer);
	//stats = *statsBuffer.getStart<PerfStats>();	
	//myPrint(stats);

	//cout<<"-------------output.size()---------------"<<output.size()<<endl;
	//PerfStats stats2 = *output.getStart<PerfStats>();
	//myPrint(stats2);
	//cout<<PerfStats::printClusterStats(&output, &output)<<endl;;
	//ProtoBuf::LogMetrics logMetrics;
	//client.getLogMetrics(locator.c_str(), logMetrics);	
	//WireFormat::ServerControlAll tmp;
	//cout<<" sizeof(ServerControlAll)"<< sizeof(WireFormat::ServerControlAll)<<endl; 
	//cout<<" sizeof(ServerControlAll)"<< sizeof(WireFormat::ServerControlAll::Response)<<endl; 
	return 0;
}	
