/*使用一个线程去专门处理所有的信号*/
#include<pthread.h>
#include<cstdio>
#include<cstdlib>
#include<iostream>
#include<unistd.h>
#include<signal.h>
#include<errno.h>

inline void handler_error_en(int en,const char* msg)
{
    errno = en;
    perror(msg);
    exit(EXIT_FAILURE);
}
bool flag = false;
static void *sig_thread(void *arg)
{
    sigset_t *set = (sigset_t *)arg;
    int s,sig;
    while(true)
    {
        s = sigwait(set,&sig);
        if(s!=0)
            handler_error_en(s,"sigwait");
        std::cout <<"signal handling got signal "<<sig<<std::endl;
        flag = true;
        break;
    }
}
int main()
{
    pthread_t thread;
    sigset_t set,org;
    int s;

    sigemptyset(&set);
    sigaddset(&set,SIGQUIT);
    sigaddset(&set,SIGINT);
    s = pthread_sigmask(SIG_BLOCK,&set,&org);
    if(s!=0)
        handler_error_en(s,"pthread_setmask");
    
    s = pthread_create(&thread,NULL,&sig_thread,(void *)&set);
    if(s!=0)
        handler_error_en(s,"pthread_create");
    
    while(flag!=true);
    if(flag)
    {
        s = pthread_sigmask(SIG_SETMASK,&org,NULL);
        if(s!=0)
            handler_error_en(s,"pthread_orgmask");
    }
    
    std::cout <<"主线程执行完了！"<<std::endl;
    pause();//等待终止信号
    // s = pthread_sigmask(SIG_BLOCK,&org,NULL);
    // if(s!=0)
        // handler_error_en(s,"pthread_orgmask");
    
}