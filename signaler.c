#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int status, x=0, value = 0;
pid_t pid[2];

void handler1(int sig){ 
    value++;
    printf("PID:%d %d\n",getpid(), value);
    kill(pid[x], SIGUSR1);
}

void handler2(int sig){ 
    value--;
    printf("PID:%d %d\n",getpid(), value);
    exit(0);
}

int main(){ 
    signal(SIGUSR1, handler1);
    if ((pid[x]=fork())==0){ 
        signal(SIGUSR1, handler2);
        kill(getppid(), SIGUSR1);
        while(1);
    }
    sleep(5);
    x++;

    if ((pid[x]= fork()) == 0){ 
        signal(SIGUSR1, handler2);
        kill(getppid(), SIGUSR1);
        while(1);
    }
    wait(&status);
    wait(&status);
    printf("%d\n", value+3);


}
