#include <sys/stat.h>
#include <sys/time.h>
#include <stdio.h>
#include <errno.h>
#include <iostream>

using namespace std;

//double difftime(time_t time1, time_t time2); //返回两个时间相差的秒数
//int gettimeofday(struct timeval* tv,struct timezone* tz);
//返回当前距离1970年的秒数和微妙数，后面的tz是时区，一般不用
void Print(const struct stat& buf){
	cout<<buf.st_size<<endl;
	cout<<ctime(&buf.st_atime);
	cout<<ctime(&buf.st_atime);
	cout<<ctime(&buf.st_mtime);
	cout<<ctime(&buf.st_ctime);
}
int main()
{
	struct stat buf;
	int res = stat("tmp.txt", &buf);
	if(res != 0)
	{
		printf("stat file fail!\n");
		printf("%d\n", errno);
	}
	Print(buf);
	struct timeval tv;
	gettimeofday(&tv, NULL);
	cout<<tv.tv_sec<<endl;
	cout<<tv.tv_usec<<endl;
	return 0;
}
