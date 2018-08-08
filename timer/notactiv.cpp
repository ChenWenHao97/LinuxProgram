#include<iostream>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<assert.h>
#include<cstdio>
#include<signal.h>
#include<unistd.h>
#include<errno.h>
#include<cstring>
#include<fcntl.h>
#include<cstdlib>
#include<sys/epoll.h>
#include<pthread.h>
#include"lst_timer.hpp"
using namespace std;
#define FD_LIMIT 65535
#define MAX_EVENT_NUMBER 1024
#define TIMESLOT 5
static int pipefd[2];
static sort_timer_lst timer_lst;
static int epollfd = 0;
int setnonblocking(int fd)
{
    int old_option = fcntl(fd,F_GETFL);
    int new_option = old_option | EPOLLET;
    fcntl(fd,F_SETFL,new_option);
    return old_option;
}
void addfd(int epollfd,int fd)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET;
    epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&event);
}
void sig_handler(int sig)
{
    int save_errno = errno;
    int msg = sig;
    send(pipefd[1],(char*)&msg,1,0);
    errno = save_errno;
}
void addsig(int sig)
{
    struct sigaction sa;
    memset(&sa,'\0',sizeof(sa));
    sa.sa_handler = sig_handler;
    sa.sa_flags |= SA_RESTART;
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig,&sa,NULL)!=-1);

}
void timer_handler()
{
    timer_lst.tick();
    alarm(TIMESLOT);//一次ａｌａｒｍ只会引发一次ｓｉｇａｌｒｍ信号xinhao
}
void cb_func(client_data * user_data)
{
    epoll_ctl(epollfd,EPOLL_CTL_DEL,user_data->sockfd,0);
    assert(user_data);
    close(user_data->sockfd);
    cout <<"close fd"<<user_data->sockfd<<endl;
}
int main(int argc,char *argv[])
{
    if(argc<=2)
    {
        cout <<"usage:"<<basename(argv[0])<<" ip_address port number\n";
        return 1;
    }
    const char * ip = argv[1];
    int port = atoi(argv[2]);

    int ret = 0;
    struct sockaddr_in address;
    bzero(&address,sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET,ip,&address.sin_addr);
    address.sin_port = htons(port);

    int listenfd = socket(PF_INET,SOCK_STREAM,0);
    assert(listenfd >=0);

    ret = bind(listenfd,(struct sockaddr*)&address,sizeof(address));
    assert(ret!=-1);

    ret = listen(listenfd,5);
    assert(ret!=-1);

    epoll_event events[MAX_EVENT_NUMBER];
    int epollfd = epoll_create(5);
    assert(epollfd!=-1);
    addfd(epollfd,listenfd);

    ret = socketpair(PF_UNIX,SOCK_STREAM,0,pipefd);
    assert(ret!=-1);
    setnonblocking(pipefd[1]);
    addfd(epollfd,pipefd[0]);

    addsig(SIGALRM);//定时器过期
    addsig(SIGTERM);//终止进程
    bool stop_server = false;

    client_data * users = new client_data[FD_LIMIT];
    bool timeout = false;
    alarm(TIMESLOT);

    while(!stop_server)
    {
        int number = epoll_wait(epollfd,events,MAX_EVENT_NUMBER,-1);
        if((number < 0) && errno!=EINTR)
        {
            cout <<"epoll failture\n";
            break;
        }
        for(int i = 0;i < number ;i++)
        {
            int sockfd = events[i].data.fd;
            if(sockfd == listenfd)
            {
                struct sockaddr_in client_address;
                socklen_t client_addrlength = sizeof(client_address);
                int connfd = accept(listenfd,(struct sockaddr*)&client_address,&client_addrlength);
                addfd(epollfd,connfd);
                users[connfd].address = client_address;
                users[connfd].sockfd = connfd;

                util_timer * timer =new util_timer();
                timer->user_data = &users[connfd];
                timer->cb_func = cb_func;
                time_t cur = time(NULL);
                timer->expire = cur + 3*TIMESLOT;
                users[connfd].timer = timer;
                timer_lst.add_timer(timer);
            }
            else if((sockfd == pipefd[0])&&(events[i].events & EPOLLIN))
            {
                int sig;
                char signals[1024];
                ret = recv(pipefd[0],signals,sizeof(signals),0);
                if(ret ==-1)
                {
                    continue;
                }
                else if(ret ==0)//什么都没收到
                {
                    continue;
                }
                else 
                {
                    for(int i = 0;i < ret;i++)
                    {
                        switch(signals[i])
                        {
                            case SIGALRM:
                            {
                                timeout = true;
                                break;
                            }
                            case SIGTERM:
                            {
                                stop_server = true;
                            }

                        }
                    }
                }


            }
            else if(events[i].events & EPOLLIN)
            {
                memset(users[sockfd].buf,'\0',BUFFER_SIZE);
                ret = recv(sockfd,users[sockfd].buf,BUFFER_SIZE-1,0);
                cout <<"get "<<ret <<"bytes of client data "<<users[sockfd].buf<<" from "<<sockfd<<endl;

                util_timer *timer = users[sockfd].timer;
                if(ret < 0)
                {
                    if(errno!=EAGAIN)
                    {
                        cb_func(&users[sockfd]);
                        if(timer)
                        {
                            timer_lst.del_timer(timer);
                        }
                    }
                }
                else if(ret == 0)
                {
                    cb_func(&users[sockfd]);
                    if(timer)
                    {
                        timer_lst.del_timer(timer);
                    }
                }
                else
                {
                    if(timer)
                    {
                        time_t cur = timer(NULL);
                        timer->expire = cur+3*TIMESLOT;
                        cout <<"adjust timer once\n";
                        timer_lst.add_timer(timer);
                    }
                }
            }
            else
            {

            }
            if(timeout)
            {
                timer_handler();
                timeout = false;
            }


        }
        close(listenfd);
        close(pipefd[1]);
        close(pipefd[0]);
        delete [] users;
        
        return 0;
    }



}

 


