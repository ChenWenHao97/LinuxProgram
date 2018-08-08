#include<iostream>
#include<unistd.h>
#include "tlpi_hdr.h"
#include<sys/select.h>
using namespace std;
static int pfd[2];
static void handler(int sig)
{
    int savedErrno;
    savedErrno = errno;

}
int main(int argc,char *argv[])
{
    fd_set  readfds;
    int ready,nfds,flags;
    struct timeval timeout;
    struct timeval *pto;
    struct sigaction sa;
    char ch;

    if(pipe(pfd)==-1)
    {
        errExit("pipe");
    }
    FD_SET(pfd[0],&readfds);
    nfds = max(nfds,pfd[0]+1);

    flags = fcntl(pfd[0],F_GETFL);
    if(flags == -1)
        errExit("fcntl-F_GETFL");
    
    flags |= O_NONBLOCK;
    if(fcntl(pfd[0],F_SETFL,flags)==-1)
        errExit("fcntl-F_SETFL");
    
    flags = fcntl(pfd[1],F_GETFL);
    if(flags==-1)
        errExit("fcntl-F_GETFL");
    flags |=O_NONBLOCK;
    if(fcntl(pfd[1],F_SETFL,flags)==-1)
        errExit("fcntl-F_SETFL");
    
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = handler;
    if(sigaction(SIGINT,&sa,NULL)==-1)
        errExit("sigaction");

    while((ready = select(nfds,&readfds,NULL,NULL,pto)==-1)&&
            errno == EINTR)
            continue;
    if(ready == -1)
        errExit("select");
    
    if(FD_ISSET(pfd[0],&readfds))
    {
        cout <<"A signal was caught"<<endl;
        while(true)
        {
            if(read(pfd[0],&ch,1)==-1)
            {
                if(errno == EAGAIN)
                    break;
                else  
                    errExit("read");
            }
        }
    }



}

