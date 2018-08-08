/*时间轮槽之间间隔足够小，定时精度越高，
效率越高，轮子的槽越多
*/
#ifndef TIME_WHEEL_TIMER
#define TIME_WHERRL_TIMER
#include <time.h>
#include <netinet/in.h>
#include <cstdio>
#include <iostream>
using namespace std;
#define BUFFER_SIZE 64
class tw_timer;
struct client_data
{
    sockaddr_in address;
    int sockfd;
    char buf[BUFFER_SIZE];
    tw_timer *timer;
};
class tw_timer
{
  public:
    int rotation;  //记录多少圈可以生效
    int time_slot; //在那个槽
    void (*cb_func)(client_data *);
    client_data *user_data;
    tw_timer *next; //下一个定时器
    tw_timer *prev; //上一个定时器
    tw_timer(int rot, int ts) : next(NULL), prev(NULL), rotation(rot), time_slot(ts) {}
};
class time_wheel
{
  private:
    static const int N = 60;
    static const int SI = 1; //每一秒转动一次
    tw_timer *slots[N];
    int cur_slot; //当前的槽
  public:
    time_wheel() : cur_slot(0)
    {
        for (int i = 0; i < N; i++)
        {
            slots[i] = NULL;
        }
    }
    ~time_wheel()
    {
        for (int i = 0; i < N; i++)
        {
            tw_timer *tmp = slots[i];
            while (tmp)
            {
                slots[i] = tmp->next;
                delete tmp;
                tmp = slots[i];
            }
        }
    }
    tw_timer *add_timer(int timeout)
    {
        if (timeout < 0)
        {
            return NULL;
        }
        int ticks = 0; //滴答次数
        if (timeout < SI)
        {
            ticks = 1; //小于一秒，记录为一秒
        }
        else
        {
            ticks = timeout / SI;
        }
        int rotation = ticks / N;
        int ts = (cur_slot + (ticks % N)) % N;
        tw_timer *timer = new tw_timer(rotation, ts);
        if (!slots[ts])
        {
            printf("add timer,rotation is %d,ts is %d,cur_slot is %d\n", rotation, ts, cur_slot);
            slots[ts] = timer;
        }
        else //加入到ｔｓ里面
        {
            timer->next = slots[ts];
            slots[ts]->prev = timer;
            slots[ts] = timer;
        }
        return timer;
    }
    void del_timer(tw_timer *timer)
    {
        if (!timer)
        {
            return;
        }
        int ts = timer->time_slot;
        if (timer == slots[ts])//头结点
        {
            slots[ts] = slots[ts]->next;
            if (slots[ts])
            {
                slots[ts]->prev = NULL;
            }
            delete timer;
        }
        else
        {
            timer->prev->next = timer->next;
            if (timer->next)
            {
                timer->next->prev = timer->prev;
            }
            delete timer;
        }
    }
    void tick()
    {
        tw_timer *tmp = slots[cur_slot];
        cout << "current slot is " << cur_slot << endl;
        while (tmp)
        {
            cout << "tick the timer once\n";
            if (tmp->rotation > 0)
            {
                tmp->rotation--;
                tmp = tmp->next;
            }
            else //过期
            {
                tmp->cb_func(tmp->user_data);
                if (tmp == slots[cur_slot])
                {
                    cout << "delete header in cur_slot\n";
                    slots[cur_slot] = tmp->next;
                    delete tmp;
                    if (slots[cur_slot])
                    {
                        slots[cur_slot]->prev = NULL;
                    }
                    tmp = slots[cur_slot];
                }
                else
                {
                    tmp->prev->next = tmp->next;
                    if (tmp->next)
                    {
                        tmp->next->prev = tmp->prev;
                    }
                    tw_timer *tmp2 = tmp->next;
                    delete tmp;
                    tmp = tmp2;
                }
            }
        }
        cur_slot = ++cur_slot % N; //更新
    }
};
#endif