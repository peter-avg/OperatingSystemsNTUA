#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>

int GATE;
char *G = "t";
char *number;
time_t start, now;

void handle_sigterm(int sig){ 
    now = 0;
    time(&now);
    // printf("[CHILD] | [PID = %d] | [TERMINATED] | [TIME = %ld] \n", getpid(), (long int)(now - start));
    exit(EXIT_FAILURE);
}

void handle_sigusr1(int sig){ 
    now = 0;
    time(&now);
    if (GATE == 1){
        GATE = 0;
    } else { 
        GATE = 1;
    }
    if (GATE == 0) { 
        printf("[GATE = %s/PID = %d/TIME = %ld] the gates are closed\n", number, getpid(), (long int)(now - start));
    } else { 
        printf("[GATE = %s/PID = %d/TIME = %ld] the gates are open\n", number, getpid(), (long int)(now - start));
    }
}

void handle_sigusr2(int sig){ 
    now = 0;
    time(&now);
    if (GATE == 0) { 
        printf("[GATE = %s/PID = %d/TIME = %ld] the gates are closed\n", number, getpid(), (long int)(now - start));
    } else { 
        printf("[GATE = %s/PID = %d/TIME = %ld] the gates are open\n", number, getpid(), (long int)(now - start));
    }
}

void handle_sigalarm(int sig){
    now = 0;
    time(&now);
    if (GATE == 0) { 
        printf("[GATE = %s/PID = %d/TIME = %ld] the gates are closed\n", number, getpid(), (long int)(now - start));
    } else { 
        printf("[GATE = %s/PID = %d/TIME = %ld] the gates are open\n", number, getpid(), (long int)(now - start));
    }

    alarm(15);

    while ( sig != -1){ 
        pause();
    }
}

void signal_handling(){
    struct sigaction sa1;
    struct sigaction sa2;
    struct sigaction sa3;
    struct sigaction sal;

    sa1.sa_handler = handle_sigterm;
    sa2.sa_handler = handle_sigusr1;
    sa3.sa_handler = handle_sigusr2;
    sal.sa_handler = handle_sigalarm;
    sigemptyset(&sal.sa_mask);
    sal.sa_flags = SA_NODEFER;

    sigaction(SIGTERM, &sa1, NULL);
    sigaction(SIGUSR1, &sa2, NULL);
    sigaction(SIGUSR2, &sa3, NULL);
    sigaction(SIGALRM, &sal, NULL);
}

int main(int argc, char **argv){ 

    signal_handling();

    if (*argv[1] == *G){ 
        GATE = 1;
    } else { 
        GATE = 0;
    }

    number = argv[2];

    if (GATE == 0) { 
        printf("[GATE = %s/PID = %d/TIME = %ld] the gates are closed\n", number, getpid(), (long int)(now - start));
    } else { 
        printf("[GATE = %s/PID = %d/TIME = %ld] the gates are open\n", number, getpid(), (long int)(now - start));
    }

    time(&start);

    alarm(15);
    pause();

    return 0;
}
