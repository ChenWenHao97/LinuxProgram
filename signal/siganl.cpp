#include<iostream>
#include<signal.h>
#include"../tlpi_hdr.h"
using namespace std;

static void sigHandler(int sig)
{
    cout <<"Ouch!"<<endl;
}
int main(int argc,char *argv[])
{
    int j;
    if(signal(SIGINT,sigHandler)==SIG_ERR)
       {
           cout <<"signal"<<endl;
           return 1;
       }
    for(j = 0;;j++)
    {
        cout <<"j:"<<j<<endl;
        sleep(2);
    }

    return 0;
}