#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) { 
    int pipe1[2];
    int pipe2[2];

    if (pipe(pipe1) == -1) {
        perror("Oops");
        exit(1);
    }

    if (pipe(pipe2) == -1) {
        perror("Oops");
        exit(1);
    }

    dup2(pipe1[0],33);
    dup2(pipe1[1],34);
    dup2(pipe2[0],53);
    dup2(pipe2[1],54);
    char *argvs[] = {"./riddle", NULL};
    execv("./riddle", argvs);
}
