#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>

char *prompt;
char *c = "tf";
pid_t pid_array[50];
pid_t wp = 0;

void handle_sigterm(int sig){ 
    for (int i = 0; i < strlen(prompt); i++) { 
        printf("[PARENT/PID = %d] Waiting for %ld children to exit\n", getpid(), strlen(prompt) - i);
        kill(pid_array[i], SIGTERM);
        printf("[PARENT/PID = %d] Child with PID = %d terminated successfully with exit status code 0\n", getpid(), pid_array[i]);
    }
    exit(EXIT_SUCCESS);
}

void handle_sigusr1(int sig){ 
    for (int i = 0; i < strlen(prompt); i++) kill(pid_array[i], SIGUSR1);
}

void handle_sigusr2(int sig){ 
    for (int i = 0; i < strlen(prompt); i++) kill(pid_array[i], SIGUSR2);
}

int check_child_health(int pid, const char *msg){ 
    if (pid < 0){ 
        perror(msg);
        return 1;
    }
    return 0;
}

pid_t create_child(char prompt, int i){
        pid_t pid = fork();

        if (check_child_health(pid, "Child failed \n") == 1){ 
            exit(EXIT_FAILURE);
        };

        if (pid == 0){ 
            char number[50];
            snprintf(number, sizeof(number), "%d", i);
            printf("[PARENT/PID=%d] Created child %d (PID=%d) and initial state '%c' \n", getppid(), i, getpid(), prompt);
            char *const argv[] = {"./child", &prompt, number, NULL};
            int status = execv("./child", argv);
        } 

        return pid;
}
    
void describe_wait_status(pid_t pid, int status) {
  if (pid < 1) {
    perror("wait() call failed");
  }

  if (pid == 0) {
    printf("Nothing happened");
  }

  if (WIFSTOPPED(status)) {
    printf("Child with PID %d stopped\n", pid);
  } else if (WIFCONTINUED(status)) {
    printf("Child with PID %d continued\n", pid);
  } else if (WIFEXITED(status)) {
    printf("Child with PID %d exited with status code %d\n", pid,
           WEXITSTATUS(status));
  } else if (WIFSIGNALED(status)) {
    printf("Child with PID %d terminated by signal %d with status code %d\n",
           pid, WSTOPSIG(status), WEXITSTATUS(status));
  }
}

void signal_handling(){ 
    struct sigaction sa1;
    struct sigaction sa2;
    struct sigaction sa3;

    sa1.sa_handler = &handle_sigterm;
    sa2.sa_handler = &handle_sigusr1;
    sigemptyset(&sa2.sa_mask);
    sa2.sa_flags = SA_NODEFER;
    sa3.sa_handler = &handle_sigusr2;
    sigemptyset(&sa3.sa_mask);
    sa3.sa_flags = SA_NODEFER;

    sigaction(SIGTERM, &sa1, NULL);
    sigaction(SIGUSR1, &sa2, NULL);
    sigaction(SIGUSR2, &sa3, NULL);
}

int main(int argc, char **argv){ 

    if (argc != 2){ 
        printf("Usage ./a.out prompt \n");
        return 1;
    }

    prompt = argv[1];

    for (int i = 0; i < strlen(prompt); i++) {
        if (prompt[i] != c[0] && prompt[i] != c[1]){
            printf("Prompt must be of proper characters");
            return 1;
        }
    }
    
    for (int i = 0; i < strlen(prompt); i++){
        pid_array[i] = create_child(prompt[i], i);
    }

    signal_handling();

    while(wp != -1){
        int status;
        wp = waitpid(-1, &status, WNOHANG | WUNTRACED);
        if (wp == 0) continue;
        if (wp == -1) {
            perror("waitpid");
            exit(EXIT_FAILURE);
        }
        if (WIFEXITED(status) || WIFSIGNALED(status)) {
            for (int i = 0; i < strlen(prompt); i++){ 
                if (wp == pid_array[i]){ 
                    printf("[PARENT/PID = %d] Child with PID = %d exited\n", getpid(), pid_array[i]);
                    pid_array[i] = create_child(prompt[i], i);
                }
            }
        }
        // wp = waitpid(-1, &status, WUNTRACED);
        if (WIFSTOPPED(status)){
            describe_wait_status(wp, status);
            kill(wp, SIGCONT);
        }
    }

    return 0;
}
