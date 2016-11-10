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

//1M 1024*1024=1048576
const static unsigned long BUFSIZE = 1048576;

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

int main(int argc, char **argv)
{
	if(argc != 2){
		cout<<"Usage "<<argv[0]<<" coordinatorLocator"<<endl;
		return -1;
	}
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
	return 0;
}	
