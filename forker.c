#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
    int i,j,status;
    pid_t child,wpid;
    for(i=0; i<2; i++)
    { printf("%d\n",i);
        child = fork();
        if(child == 0)
        {
            printf("Child: %d, Father: %d with i = %d\n",getpid(),getppid(),i);
            for(j=0; j<2; j++)
            {
                child=fork();
                if(child == 0)
                {
                    printf("Child: %d, Father: %d inside loop\n",getpid(),getppid());
                    exit(0);
                }
            }
            while((wpid = wait(&status)) > 0);
        }    
  }
    while((wpid = wait(&status)) > 0); 
    return 0;
}
