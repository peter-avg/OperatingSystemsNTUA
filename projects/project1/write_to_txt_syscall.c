#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/stat.h>

bool file_exists(const char *filename)
{
    struct stat buffer;
    return stat(filename, &buffer) == 0 ? true : false;
}

int main(int argc, char* argv[]){

    if (argc == 1){ 
        printf("Usage ./a.out output.txt\n");
        return 1;
    }

    char *prompt = strdup(argv[1]);

    if (strcmp( prompt, "--help") == 0 || argc != 2){ 
        printf("Usage ./a.out output.txt\n");
        return 1;
    }

    if (file_exists(prompt)){ 
        printf("Error: output.txt already exists");
        return 1;
    }

    int fd = open(argv[1], O_CREAT | O_WRONLY, 0644);

    int status;
    char buffer[50] = "";
    int j; 
    int max_len = sizeof buffer;

    pid_t child=fork();

    if (child < 0){ 
        return 1;
    }

    if (child == 0){ 
        j = snprintf(buffer, max_len, "[CHILD] getpid()=%d, getppid()=%d \n", getpid(), getppid());
        if (j>0) write(fd, buffer, max_len);
    }
    else {
        wait(&status);
        j = snprintf(buffer, max_len, "[PARENT] getpid()=%d, getppid()=%d \n", getpid(), getppid());
        if (j>0) write(fd, buffer, max_len);
    }

    close(fd);
    free(prompt);

    return 0;
}
