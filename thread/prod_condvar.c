#include <time.h>
#include<unistd.h>
#include <stdio.h>
#include<stdlib.h>
#include <pthread.h>

static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

static int avail = 0;

static void *producer(void *arg)
{
    int cnt = atoi((char *)arg);
    int s, j;
    // printf("进入一个线程\n");
    for (j = 0; j < cnt; j++)
    {
        sleep(1);

        /* 省略一部分内容*/

        s = pthread_mutex_lock(&mtx);
        if (s != 0)
            perror("pthread_mutex_lock");

        avail++; /* 让消费者知道是可用的 */

        s = pthread_mutex_unlock(&mtx);
        if (s != 0)
            perror("pthread_mutex_unlock");

        s = pthread_cond_signal(&cond);
         /* 唤醒消费者 */
        if (s != 0)
            perror("pthread_cond_signal");
    }
    // printf("返回一个线程\n");
    return NULL;
}

int main(int argc, char *argv[])
{
    pthread_t tid;
    int s, j;
    int totRequired; /* 需要的个数 */
    int numConsumed; /* 消耗的个数 */
    int done;        //标志什么时候结束
    time_t t;

    t = time(NULL);

    /* 创建所有线程*/
    printf("创建之前\n");////////////
    totRequired = 0;
    for (j = 1; j < argc; j++)
    {
        totRequired += atoi(argv[j]);
        // printf("创建一个线程\n");
        s = pthread_create(&tid, NULL, producer, argv[j]);
        if (s != 0)
            perror("pthread_create");
    }

    /* 循环去消耗可以用的部分 */

    numConsumed = 0;
    done = 0;

    for (;;)
    {
        s = pthread_mutex_lock(&mtx);
        if (s != 0)
            perror("pthread_mutex_lock");
        // printf("锁住消费者\n");
        while (avail == 0)
        { /* 如果没有可以消耗的，就等，知道条件满足 */
            s = pthread_cond_wait(&cond, &mtx);
            if (s != 0)
                perror("pthread_cond_wait");
        }

        /* 此时有锁 */

        while (avail > 0)
        { /* 在加锁的时候消耗掉所有可以用的东西*/

            /*可以干别的事情 */

            numConsumed++;
            avail--;
            printf("T=%ld: numConsumed=%d\n", (long)(time(NULL) - t),numConsumed);

            done = numConsumed >= totRequired ? 1 : 0; //消耗的大于等于需要的，就准备退出
        }

        s = pthread_mutex_unlock(&mtx);
        if (s != 0)
            perror("pthread_mutex_unlock");

        if (done)
            break;

        /* 做其他事情 */
    }

    return 0;
}
