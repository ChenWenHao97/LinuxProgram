#include<unistd.h>
#include<iostream>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
using namespace std;
int main()
{
    int fd = open("1.txt",O_RDWR);
    int oldflag = fcntl(fd,F_GETFL);
    off_t cur = lseek(fd,0,SEEK_END);
    cout <<cur<<endl;
    int newfd = dup(fd);
    off_t newcur = lseek(newfd,0,SEEK_CUR);
    cout <<newcur<<endl;
    int newflag = fcntl(newfd,F_GETFL);
    if(cur == newcur)
        cout <<"position right"<<endl;
    else
        cout <<"position wrong"<<endl;
    if(oldflag==newflag)
        cout <<"flag right"<<endl;
    else    
        cout <<"flag wrong"<<endl;
}