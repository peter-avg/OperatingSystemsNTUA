#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

time_t start, now;

void handler(int sig){ 
    now = 0;
    time(&now);
    printf("TIME: %ld \n", (long int)(now - start));
    alarm(5);
}

int main(){ 
    struct sigaction sa;
    sa.sa_handler = &handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    sigaction(SIGALRM, &sa, NULL);
    time(&start);

    printf("TIME: %d \n", 0);

    alarm(5);

    while(1){
        pause();
    }

    return 0;
}
