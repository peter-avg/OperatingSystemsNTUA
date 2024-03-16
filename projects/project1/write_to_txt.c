#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/stat.h>

bool file_exists (char *filename) {
  struct stat buffer; 
  return (stat (filename, &buffer) == 0); 
}

int main(int argc, char* argv[]){ 

    if (argc == 1){ 
        printf("Usage ./a.out output.txt\n");
        exit(1);
    }

    char *prompt = strdup(argv[1]);

    if (strcmp( prompt, "--help") == 0 || argc != 2){ 
        printf("Usage ./a.out output.txt\n");
        exit(1);
    }

    if (file_exists(prompt)) { 
        printf("Error: %s already exists\n", prompt);
        exit(1);
    }

    FILE *fp;
    int status;
    fp = fopen(prompt, "w");
    pid_t child = fork();

    if (child < 0){ 
        exit(1);
    }

    if (child == 0){ 
        fprintf(fp,"[CHILD] getpid()=%d, getppid()=%d\n", getpid(), getppid());
    }
    else {
        wait(&status);
        fprintf(fp,"[PARENT] getpid()=%d, getppid()=%d\n", getpid(), getppid());
    }
    fclose(fp);

    free(prompt);

    exit(0);
}


