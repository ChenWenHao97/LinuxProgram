#include <signal.h>
#include "../head/curr_time.h"                  /* Declaration of currTime() */
#include "../head/tlpi_hdr.h"
// #include "./head/error_functions.h"
#include<stdio.h>
#define SYNC_SIG SIGUSR1                /* Synchronization signal */
#define errExit perror 

static void             /* Signal handler - does nothing but return */
handler(int sig)
{
    printf("catch signal\n ");
}

int
main(int argc, char *argv[])
{
    pid_t childPid;
    sigset_t blockMask, origMask, emptyMask;
    struct sigaction sa;

    setbuf(stdout, NULL);               /* Disable buffering of stdout */

    sigemptyset(&blockMask);
    sigaddset(&blockMask, SYNC_SIG);    //加入阻塞信号
    if (sigprocmask(SIG_BLOCK, &blockMask, &origMask) == -1)//进行阻塞，记录旧信号信息
        errExit("sigprocmask");

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;//恢复到上一次中断的地方
    sa.sa_handler = handler;
    if (sigaction(SYNC_SIG, &sa, NULL) == -1)
        errExit("sigaction");

    switch (childPid = fork()) {
    case -1:
        errExit("fork");

    case 0: /* Child */

        /* Child does some required action here... */

        printf("[%s %ld] Child started - doing some work\n",
                currTime("%T"), (long) getpid());
        sleep(2);               /* Simulate time spent doing some work */

        /* And then signals parent that it's done */

        printf("[%s %ld] Child about to signal parent\n",
                currTime("%T"), (long) getpid());
        if (kill(getppid(), SYNC_SIG) == -1)
            errExit("kill");
        //父子进程都有sigaction，所以专门给父进程发送过去，让他去执行处理函数

        /* Now child can do other things... */

        _exit(EXIT_SUCCESS);//使用_exit,更安全一些,会清理一些缓冲

    default: /* Parent */

        /* Parent may do some work here, and then waits for child to
           complete the required action */

        printf("[%s %ld] Parent about to wait for signal\n",
                currTime("%T"), (long) getpid());
        sigemptyset(&emptyMask);
        if (sigsuspend(&emptyMask) == -1 && errno != EINTR)
            errExit("sigsuspend");
        //函数在这里pasue，收到信号就停止，restart标志可以让他继续执行
        printf("[%s %ld] Parent got signal\n", currTime("%T"), (long) getpid());

        /* If required, return signal mask to its original state */

        if (sigprocmask(SIG_SETMASK, &origMask, NULL) == -1)
            errExit("sigprocmask");

        /* Parent carries on to do other things... */

        exit(EXIT_SUCCESS);
    }
}