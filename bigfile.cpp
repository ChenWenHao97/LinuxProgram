#define _FILE_OFFSET_BITS 64

#include<sys/stat.h>
#include<fcntl.h>
#include<iostream>
#include<cstring>
#include<cstdio>
#include<cstdlib>
#include <sys/types.h>
#include <unistd.h>
using namespace std;
int main(int argc,char * argv[])
{
    int fd;
    off_t off;
    if(argc!=3||strcmp(argv[1],"--help)==0"))
        cout << argv[0]<<endl;

        fd = open(argv[1],O_RDWR|O_CREAT,S_IRUSR|S_IWUSR);
        if(fd == -1)
            cout <<"open"<<endl;
        off = atoll(argv[2]);
        if(lseek(fd,off,SEEK_SET)==-1)
            cout << "lseek"<<endl;
        
        if(write(fd,"test",4)==-1)
            cout <<"write"<<endl;

        return 0;
}