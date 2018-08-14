#include<signal.h>
#include<libgen.h>
#include"../head/tlpi_hdr.h"
#include<stdio.h>
#include<unistd.h>
#define CMD_SIZE 200
int main(int argc,char *argv[])
{
    char cmd[CMD_SIZE];
    pid_t childPid;

    setbuf(stdout,NULL);
    printf("Parent PID=%ld\n",(long)getpid());

    switch(childPid = fork())
    {
        case -1:
            perror("fork");
        case 0:
            printf("Child (PID = %ld) exiting\n",(long)getpid());
            _exit(EXIT_SUCCESS);
        default:
            sleep(3);
            snprintf(cmd,CMD_SIZE,"ps | grep %s",basename(argv[0]));
            cmd[CMD_SIZE-1] = '\0';
            system(cmd);

            if(kill(childPid,SIGKILL) == -1)
                perror("kill");
            sleep(3);
            printf("After sending SIGKILL to zombie (PID=%ld):\n",(long)childPid);
            system(cmd);

    }
    exit(EXIT_SUCCESS);
}