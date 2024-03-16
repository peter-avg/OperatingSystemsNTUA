#include <signal.h>
// #include <errno.h>
#include <unistd.h>
#include <string.h>
// #include <fcntl.h>
#include <stdio.h>
#include <fcntl.h>
// #include <sys/wait.h>
#include <sys/select.h>
#include <stdlib.h>
#include <ctype.h>

char *prompt = "--round-robin";
pid_t wp = 0;
pid_t pid_array[50];
pid_t send_to = 0;
int iterator = 0;

#define BUFSIZE 256;
 
int check_argument_errors(int argc, char **argv) { 
    if (argc > 3 || argc < 2) { 
        printf("Usage: ask3 <nChildren> [--random] [--round-robin]\n");
        return 1;
    }

    if (argc == 2) { 
        for (int i=0; i<strlen(argv[1]); i++) {
            if (isdigit(argv[1][i]) == 0) {
                printf("Usage: ask3 <nChildren> [--random] [--round-robin]\n");
                return 1;
            }
        }
    }

    if (argc == 3) { 
        for (int i=0; i<strlen(argv[1]); i++) {
            if (isdigit(argv[1][i]) == 0) {
                printf("Usage: ask3 <nChildren> [--random] [--round-robin]\n");
                return 1;
            }
        }
        if (strcmp(argv[2], "--round-robin") && strcmp(argv[2], "--random")){ 
            printf("Usage: ask3 <nChildren> [--random] [--round-robin]\n");
            return 1;
        }
    prompt = argv[2];
    }
    
    return 0;
}

int check_input(char *input, int digits, int n) { 

    if (n > 0 && input[n-1] == '\n') {
        input[n-1] = '\0';
    }

    if (n <= 5  && strncmp(input, "help", 4) == 0) {
        printf("Type a number to send a job to a child\n");
        return 1;
    }
    else if (n <= 5 && strncmp(input, "exit", 4) == 0) {
        for (int i = 0; i < (int)digits; i++) { 
            printf("Killing Child with PID = %d\n", pid_array[i]);
            kill(pid_array[i], SIGTERM);
        }
        exit(EXIT_SUCCESS);
        return 0;
    }
    else
    {
        for (int i=0; i<strlen(input); i++) {
            if (isdigit(input[i]) == 0) {
                printf("Type a number to send a job to a child\n");
                return 1;
            } 
            return 0;
        }
    }
    return 1;
}

int check_child_health(int pid){ 
    if (pid < 0){ 
        perror("Fork failed\n");
        return 1;
    }
    return 0;
}

int main(int argc, char **argv) { 

    if (check_argument_errors(argc, argv) == 1) exit(EXIT_FAILURE); 

    fd_set read_fds;
    int digits = strtol(argv[1], NULL, 10);
    int (*in)[2] = malloc(digits * sizeof(*in));
    int (*out)[2] = malloc(digits * sizeof(*out));

    for (int i = 0; i < digits; i++) {
        if (pipe(in[i]) == -1) { 
            printf("There's been a issue with your pipes!\n");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < digits; i++) {
        if (pipe(out[i]) == -1) { 
            printf("There's been a issue with your pipes!\n");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < digits; i++) {
        pid_t pid = fork();
        pid_array[i] = pid;

        if (check_child_health(pid) == 1){ 
            printf("Child failed to be born!\n");
            exit(EXIT_FAILURE);
        };

        if (pid == 0){ 
            int buffer;
            printf("Child has been born\n");


            while(1) { 

                if (read(in[i][0], &buffer, sizeof(int)) < 0){ 
                    printf("Reading failed in child\n");
                    continue;
                } 
                printf("[Child] %d received data: %d\n", i, buffer);
                buffer++;
                sleep(5);
                ssize_t written = write(out[i][1], &buffer, sizeof(int));
            }
        }
    }


    int digits2;


    while (1) { 

        //initializing the file descriptor set for select 
        FD_ZERO(&read_fds);

        //initializing the maximum number of file descriptor
        int max_fd = -1;


        //finding the actual maximum number of file descriptor
        //and adding each pipe file descriptor to the set.
        for (int i = 0; i < digits; i++) { 
            FD_SET(out[i][0], &read_fds);
            if (out[i][0] > max_fd) { 
                max_fd = out[i][0];
            }
        }

        FD_SET(STDIN_FILENO, &read_fds);
        if (STDIN_FILENO > max_fd) { 
            max_fd = STDIN_FILENO;
        }

        int ready = select(max_fd + 1, &read_fds, NULL, NULL, NULL); 

        if (ready < 0) { 
            printf("select failed!\n");
            continue;
        }

        if(FD_ISSET(STDIN_FILENO, &read_fds)){ 
            ssize_t buffer_size = 1024; 
            char *input = malloc(buffer_size);

            int bytes_read = read(STDIN_FILENO, input, buffer_size);

            if (bytes_read < 0) { 
                continue;
            }

            input[bytes_read] = '\0';

            if (check_input(input, digits, bytes_read) == 1) { 
                free(input);
                continue;
            }  

            if (!strcmp(prompt, "--random")) { 
                send_to = pid_array[rand() % (digits)];
            }

            if (!strcmp(prompt, "--round-robin")) { 
                send_to = pid_array[iterator % (digits)];
                iterator++;
            }

            digits2 = atoi(input); 

            for (int i = 0; i < digits; i++) {
                if (send_to == pid_array[i]) { 
                    printf("Writing %d to Child %d \n", digits2, i);
                    if (write(in[i][1], &digits2, sizeof(int)) < 0) { 
                        printf("Write to child failed\n");
                        continue;
                    }
                }

            }

        }

        for ( int i = 0; i < digits; i++) { 
            if(FD_ISSET(out[i][0], &read_fds)) { 
                if (read(out[i][0], &digits2, sizeof(int)) < 0) { 
                    printf("failed to read from child\n");
                    continue;
                }
                printf("[parent] got %d from child %d\n", digits2, i);
            }
        }
        

        // FD_CLR(STDIN_FILENO, &read_fds);
    }

    return 0;

}

