#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

int main(int argc, char **argv) { 

    int read_bytes;
    char buffer[100];

    int fd = openat(AT_FDCWD, "secret_number", O_RDWR);

    for(;;) {
        if ((read_bytes = read(fd, buffer, 16)) == -1) {
            perror("Oopsies\n");
            exit(EXIT_FAILURE);
        }

        if (read_bytes > 0) {
            printf("%s",buffer);

        }
    }

    printf("\n");
    exit(EXIT_SUCCESS);

}
