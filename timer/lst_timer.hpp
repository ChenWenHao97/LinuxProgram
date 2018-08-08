//升序定时器链表
#ifndef LST_TIMER
#define LST_TIMER
#include<time.h>
#define BUFFER_SIZE 64
class util_tmer;

struct client_data{
    sockaddr_in address;
    int sockfd;
    char buf[BUFFER_SIZE];
    util_timer *timer;
};
class util_timer{
    public:
        time_t expire;//timeout
        void (*cb_func)(client_data*);
        client_data * user_data;
        util_timer* prev;//前一个定时器
        util_timer* next;//下一个定时器
        util_timer():prev(NULL),next(NULL){}
};
class sort_timer_lst{
    public:
        sort_timer_lst():head(NULL),tail(NULL){}
        ~sort_timer_lst()
        {
            util_timer *tmp = head;
            while(tmp)
            {
                head = tmp->next;
                delete tmp;
                tmp = head;
            }
        }
        void add_timer(util_timer* timer)
        {
            if(!timer)
            {
                return;
            }
            if(!head)
            {
                head = tail = timer;
                return;
            }
            if(head->expire < head->expire)
            {
                timer->next =head;
                head->prev = timer;
                head = timer;
                return;
            }
            add_timer(timer,head);
        }
        void adjust_timer(util_timer *timer)
        {
            if(!timer)
            {
                return;
            }
            util_timer *tmp = timer->next;
            if(!tmp || (timer->expire < tmp->expire))
            {
                return;
            }
            if(timer == head)
            {
                head =head->next;
                head->prev = NULL:
                timer->next = NULL;
                add_timer(timer,head);
            }
            else//不在头，就取出来
            {
                timer->prev->next = timer->next;
                timer->next->prev = timer->prev;
                add_timer(timer,timer->next);
            }
        }
        void del_timer(util_timer * timer)
        {
            if(!timer)
            {
                return;
            }
            if((timer == head)&&(timer == tail))//只有一个定时器的情况
            {
                delete timer;
                head = tail = NULL;
                return;
            }
            if(timer == head)
            {
                head = head->next;
                head->prev = NULL;
                delete timer;
                return;
            }
            if(timer ==tail)
            {
                tail = tail->prev;
                tail->next = NULL;
                delte timer;
                return;
            }
            timer->prev->next = timer->next;//位于中间
            timer->next->prev = timer->prev;
            delete timer;
        }
        void tick()
        {
            if(!head)
            {
                return;
            }
            cout <<"timer tick"<<endl;
            timer_t cur = time(NULL);//获取系统时间
            util_timer * tmp = head;
            while(tmp)
            {
                if(cur < tmp->expire)
                    break;
            
                tmp->cb_func(tmp->user_data);
                head = tmp->next;
                if(head)
                {
                    head->prev =NULL;
                }
                delete tmp;
                tmp = head;
            }
            
        }
    
    private:
        util_timer* head;
        util_timer* tail;
        void add_timer(util_timer* timer,util_timer* lst_head)
        {
            util_timer* prev = lst_head;
            util_timer* tmp = prev->next;
            while(tmp)
            {
                if(timer->expire < tmp->expire)
                {
                    prev->next =timer;
                    timer->next =tmp;
                    tmp->prev = timer;
                    timer->prev = prev;
                    break;
                }
                prev = tmp;
                tmp = tmp->next;
            }
            if(!tmp)
            {
                prev->next = timer;
                timer->prev = prev;
                timer->next = NULL;
                tail = timer;
            }

        }



};
#endif

