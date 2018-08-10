//不可重入的例子
#define _XOPEN_SOURCE 600
#include<iostream>
#include<unistd.h>
#include<signal.h>
#include<cstring>
#include"../tlpi_hdr.h"

static char *str2;
static int handled = 0;
using namespace std;
static void handler(int sig)
{
    crypt(str2,"xx");
    handled++;
}
int main(int argc,char* argv[])
{
    char *cr1;
    int callNum,mismatch;
    struct sigaction sa;

    if(argc!=3)
    {
        cout<<argv[0]<<"str1 str2\n"<<endl;
    }
    str2 = argv[2];
    cr1 = strdup(crypt(argv[1],"xx"));
    if(cr1 == NULL)
    {
       cout<<"strdup"<<endl;
       return 1;
    }
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = handler;
    if(sigaction(SIGINT,&sa,NULL)==-1)
    {
       cout<<"sigaction"<<endl;
       return 1;
    }
    for(callNum = 1,mismatch = 0; ;callNum++)
    {
        if(strcmp(crypt(argv[1],"xx"),cr1)!=0)//处理了handler之后，crypt带回来的值已经改变了
        {
            mismatch++;
            cout <<"Mismatch on call "<<callNum<<"(mismatch="<<mismatch<<" handled="<<handled<<endl;
        }
    }
    return 0;
}
