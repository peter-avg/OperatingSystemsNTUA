#include <stdlib.h> 
#include <stdio.h> 
#include <fcntl.h> 
#include <unistd.h> 

int main (int argc, char** argv) {
    pid_t pd = getpid();

    while (pd !=32767){
        pd = fork();
        if (pd == 0) {
            if (getpid() == 32767){
                execve("./riddle",NULL,NULL);
            }
        }
    }
    return 0 ; 
} 
