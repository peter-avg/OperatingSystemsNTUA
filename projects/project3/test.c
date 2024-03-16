#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv) { 
    int iterator = 0;
    int pid_array[4] = {0, 1, 2, 3};
    int send_to;

    while(1) { 
        send_to = pid_array[iterator % 4];
        printf("Sending to child %d\n", send_to);
        iterator++;
        sleep(2);
    }

}
